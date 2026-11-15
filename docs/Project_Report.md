# Programming Languages and Compilers
## Phase 1: Lexical Analyzer Generator — Project Report
**Alexandria University — Faculty of Engineering**  
**Computer and Systems Engineering Department**  

---

## Table of Contents
1. [Executive Summary & System Architecture](#1-executive-summary--system-architecture)
2. [Section A: Description of Data Structures](#2-section-a-description-of-data-structures)
3. [Section B: Algorithms & Techniques Used](#3-section-b-algorithms--techniques-used)
4. [Section C: Resultant Transition Table for Minimal DFA](#4-section-c-resultant-transition-table-for-minimal-dfa)
5. [Section D: Resultant Stream of Tokens for Example Program](#5-section-d-resultant-stream-of-tokens-for-example-program)
6. [Section E: Assumptions Made and Justifications](#6-section-e-assumptions-made-and-justifications)
7. [Step-by-Step Implementation Status & Test Verification](#7-step-by-step-implementation-status--test-verification)

---

## 1. Executive Summary & System Architecture

The objective of Phase 1 is to build an automatic **Lexical Analyzer Generator** in C++. The tool reads language token definitions written in regular expressions, constructs Non-deterministic Finite Automata (NFAs), merges them into a Combined NFA, converts it to a Deterministic Finite Automaton (DFA), minimizes the DFA, and outputs the resulting transition table. It also simulates this DFA as a runtime scanner that matches input source code using the **Maximal Munch (longest prefix)** rule, manages a Symbol Table for identifiers, and integrates with the Phase 2 Parser via `getNextToken()`.

```mermaid
flowchart TD
    A["Lexical Rules Text File\n(Definitions, Regexes, Keywords, Punctuations)"] --> B["Step 0: RuleParser\n(Macro Expansion, Range Expansion, Infix to Postfix)"]
    B --> C["Step 1: NFA Engine\n(Thompson's Construction & Combined NFA)"]
    C --> D["Step 2: DFA Engine\n(Subset Construction & Priority Resolution)"]
    D --> E["Step 3: DFA Minimizer\n(Partition Refinement / Hopcroft's)"]
    E --> F["Transition Table Exporter"]
    F --> G["Step 4: Lexical Analyzer Simulator\n(Maximal Munch, Symbol Table, Error Recovery)"]
    H["Input Source Code Program"] --> G
    G --> I["Recognized Tokens Stream File"]
    G --> J["Symbol Table (Identifiers)"]
    G -.-> K["Phase 2 Parser (getNextToken API)"]
```

---

## 2. Section A: Description of Data Structures

### 2.1 Token Representation (`TokenType` & `RegexToken`)
Inside [`include/RegexToken.h`](../include/RegexToken.h), regular expressions are converted from raw text into a strongly typed token stream:
```cpp
enum class TokenType {
    LITERAL,    // Characters to match directly (e.g. 'a', '0', '+', '=')
    LAMBDA,     // \L (empty string / epsilon)
    CONCAT,     // Explicit concatenation operator (internal)
    UNION,      // |
    STAR,       // * (Kleene closure: 0 or more)
    PLUS,       // + (Positive closure: 1 or more)
    LPAREN,     // (
    RPAREN      // )
};

struct RegexToken {
    TokenType type;
    char value; // Meaningful when type == LITERAL
};
```
* **Why this structure**: It cleanly separates operators from literal characters. For instance, an escaped plus `\+` becomes `RegexToken(TokenType::LITERAL, '+')`, while a repeat plus `+` becomes `RegexToken(TokenType::PLUS, '+')`. This eliminates any ambiguity during automata construction.

### 2.2 Token Rule Definition (`TokenRule`)
Inside [`include/RuleParser.h`](../include/RuleParser.h), each language token is packaged with its priority and parsed representations:
```cpp
struct TokenRule {
    std::string name;                   // e.g. "id", "num", "relop", "int", ";"
    int priority;                       // Lower number = higher priority
    std::string rawRegex;               // Original expression from file
    std::string expandedRegex;          // After macro and range expansions
    std::vector<RegexToken> infix;      // Infix tokens with explicit CONCAT
    std::vector<RegexToken> postfix;    // Postfix (RPN) tokens for Thompson's
};
```

### 2.3 Automata Data Structures (`NFAState`, `NFAFragment`, and `NFA`)
Inside [`include/NFA.h`](../include/NFA.h) and [`include/NFABuilder.h`](../include/NFABuilder.h), the NFA graph is represented as an adjacency-list based structure:
```cpp
using namespace std;

struct NFAState {
    int id;
    bool isAccepting = false;
    string tokenName = "";   // e.g. "id", "num", "int", ";"
    int priority = -1;       // Lower number = higher priority

    // Character transitions: char -> set of target state IDs
    map<char, set<int>> transitions;

    // Epsilon / Lambda transitions: set of target state IDs
    set<int> epsilonTransitions;
};

// Represents a sub-automaton during Thompson's construction
struct NFAFragment {
    int start;
    int accept;
};

class NFA {
public:
    int startStateId = 0;
    vector<NFAState> states;
    set<char> alphabet; // All unique input characters recognized by this NFA

    int addState();
    void addTransition(int from, char input, int to);
    void addEpsilonTransition(int from, int to);
    void setAccepting(int stateId, const string& tokenName, int priority);
    vector<int> getAcceptingStates() const;
};
```
* **Why this structure**:
  * `NFAFragment` tracks the entry and exit of each sub-expression on the evaluation stack.
  * `map<char, set<int>>` and `set<int>` provide clean, deterministic iteration and fast lookups for both symbol transitions and $\epsilon$-closures.
  * Every state has a globally unique ID across the entire NFA, eliminating the need for state re-indexing or graph merging.

### 2.4 DFA State & Transition Table (`DFAState` and `DFA`)
Inside [`include/DFA.h`](../include/DFA.h), the DFA mirrors the NFA structure but allows **exactly one** target per character:
```cpp
struct DFAState {
    int id;
    bool isAccepting = false;
    string tokenName = "";   // Winning token class (e.g. "id", "num", "if")
    int priority = -1;       // Priority of the winning token (lower = higher priority)

    // Deterministic transitions: character -> exactly one target state ID.
    // A missing entry means "no transition" (implicit dead state).
    map<char, int> transitions;

    // The set of NFA state IDs this DFA state represents (filled by subset construction).
    // Empty for states of a minimized DFA.
    set<int> nfaStates;
};

class DFA {
public:
    static const int NO_STATE = -1;   // Returned when there is no transition

    int startStateId = 0;
    vector<DFAState> states;
    set<char> alphabet;               // All unique input characters used by this DFA

    int addState();
    void addTransition(int from, char input, int to);
    void setAccepting(int stateId, const string& tokenName, int priority);
    int getNextState(int stateId, char input) const;      // NO_STATE if no move
    bool isAccepting(int stateId) const;
    string getAcceptingToken(int stateId) const;
    vector<int> getAcceptingStates() const;
    string matchWhole(const string& input) const;         // whole-string match (used by tests)

    void printTransitionTable(ostream& out) const;
    bool saveTransitionTableCSV(const string& filePath) const;
    bool saveTransitionTableText(const string& filePath) const;
};
```
* **Why this structure**:
  * `map<char, int>` (instead of the NFA's `map<char, set<int>>`) enforces determinism: one character leads to at most one state.
  * A **missing** map entry means "no move". This is an *implicit dead state*: it keeps the table small and is exactly what the Step 4 scanner needs to know when to stop and fall back to the last accepting state.
  * `nfaStates` stores the group of NFA states that a DFA state stands for, so subset construction can recognise a group it has already seen. It is empty after minimization.
  * `getNextState`, `isAccepting` and `getAcceptingToken` are the only functions the Step 4 scanner needs to simulate the machine.
* **Transition table**: conceptually a 2D mapping `table[stateId][inputChar] -> nextStateId`. It is stored sparsely in each state's `transitions` map and exported as a dense matrix (rows = states, columns = alphabet) by `printTransitionTable`, `saveTransitionTableText` and `saveTransitionTableCSV`.

### 2.5 Token & Symbol Table Data Structures (`Token`, `SymbolEntry`, `SymbolTable`)
Inside [`include/Token.h`](../include/Token.h) and [`include/SymbolTable.h`](../include/SymbolTable.h):
```cpp
struct Token {
    string tokenClass;     // e.g. "int", "id", "num", ";", "relop", "EOF"
    string attributeValue; // Matched lexeme value (e.g. "sum", "10", "!=")
    int line = 1;
    int column = 1;

    bool isEOF() const { return tokenClass == "EOF"; }
};

struct SymbolEntry {
    string name;       // Identifier name/lexeme
    string tokenClass; // Token type ("id")
    int firstLine = 1; // First line where identifier appeared
    int occurrences = 1;
};

class SymbolTable {
private:
    unordered_map<string, SymbolEntry> table;
    vector<string> insertionOrder;

public:
    bool insert(const string& name, const string& tokenClass = "id", int line = 1);
    bool contains(const string& name) const;
    const SymbolEntry* get(const string& name) const;
    vector<SymbolEntry> getAllEntries() const;
    void print(ostream& out) const;
};
```
* **Why this structure**:
  * Provides $O(1)$ average time lookups for identifier duplicate checking via hash map.
  * Maintains an `insertionOrder` vector to display identifiers deterministically in the order they were first introduced in source code.
  * Tracks identifier occurrence counts and source line references.

### 2.6 Lexical Analyzer Runtime Simulator (`LexicalAnalyzer`)
Inside [`include/LexicalAnalyzer.h`](../include/LexicalAnalyzer.h):
```cpp
class LexicalAnalyzer {
private:
    DFA dfa;
    string input;
    size_t cursor = 0;
    int currentLine = 1;
    int currentColumn = 1;

    SymbolTable symbolTable;
    vector<string> errorLog;
    ostream* errorStream;

    void skipWhitespace();
    void recoverFromError(char badChar, int line, int col);

public:
    LexicalAnalyzer(const DFA& minimizedDfa, const string& sourceCode, ostream* errStream = &cerr);

    Token getNextToken();           // Phase 2 Parser interface
    bool hasMoreTokens();
    vector<Token> tokenizeAll();
    const SymbolTable& getSymbolTable() const;
};
```

---

## 3. Section B: Algorithms & Techniques Used

### 3.1 Lexical Specification Parsing & Priority Assignment
* **Line Categorization**: Lines are identified as definitions (`=`), expressions (`:`), keywords (`{ }`), or punctuations (`[ ]`).
* **Reserved Priority**: Keywords are reserved words in the language and are assigned the highest priority levels ($0 \dots K-1$), followed by punctuations, followed by regular expressions in file appearance order.

### 3.2 Macro Definition & Range Expansion
* **Macro Substitution**: Regular definitions (e.g. `digit = 0 - 9`, `digits = digit+`) are expanded topologically and substituted into subsequent definitions and expressions using whole-word boundary matching.
* **Range Expansion**: Ranges like `a-z`, `A-Z`, and `0 - 9` are recognized and expanded into explicit character unions `(a|b|...|z)` and `(0|1|...|9)`, simplifying downstream Thompson's construction.

### 3.3 Explicit Concatenation Insertion
In standard regular expressions, concatenation is implicit (juxtaposition). Before conversion to postfix, an explicit `CONCAT` token is inserted between any adjacent pair of tokens where:
$$\text{prev} \in \{\text{LITERAL}, \text{LAMBDA}, \text{RPAREN}, \text{STAR}, \text{PLUS}\} \quad\text{and}\quad \text{curr} \in \{\text{LITERAL}, \text{LAMBDA}, \text{LPAREN}\}$$

### 3.4 Infix to Postfix Conversion (Dijkstra's Shunting-Yard)
To make Thompson's NFA construction a straightforward linear stack evaluation, infix expressions are converted to Postfix notation with the following precedence:
1. **Precedence 3 (Highest)**: Unary postfix operators (`*`, `+`) — emitted directly to output.
2. **Precedence 2**: Binary `CONCAT` (`.`) — left-associative.
3. **Precedence 1**: Binary `UNION` (`|`) — left-associative.
4. **Precedence 0**: Parentheses `(` and `)`.

### 3.5 Thompson's NFA Construction & Combined Master NFA
Each regular expression (in postfix notation) is evaluated linearly using a stack of `NFAFragment`s according to Ken Thompson's inductive construction:

1. **Literal Character (`c`)**: Creates two states $s$ and $a$, and a transition $s \xrightarrow{c} a$. Returns fragment $(s, a)$.
2. **Lambda ($\Lambda$ / $\epsilon$)**: Creates two states $s$ and $a$, and an $\epsilon$-transition $s \xrightarrow{\epsilon} a$. Returns fragment $(s, a)$.
3. **Concatenation ($A \cdot B$)**: Pops $B$, pops $A$. Connects $A$'s accept state to $B$'s start state via an $\epsilon$-transition: $A.\text{accept} \xrightarrow{\epsilon} B.\text{start}$. Returns fragment $(A.\text{start}, B.\text{accept})$.
4. **Union ($A \mid B$)**: Pops $B$, pops $A$. Creates a new start state $s$ and new accept state $a$. Adds $\epsilon$-transitions $s \xrightarrow{\epsilon} A.\text{start}$, $s \xrightarrow{\epsilon} B.\text{start}$, $A.\text{accept} \xrightarrow{\epsilon} a$, and $B.\text{accept} \xrightarrow{\epsilon} a$. Returns fragment $(s, a)$.
5. **Kleene Closure ($A^*$)**: Pops $A$. Creates new start $s$ and accept $a$. Adds $\epsilon$-transitions $s \xrightarrow{\epsilon} A.\text{start}$, $A.\text{accept} \xrightarrow{\epsilon} a$, loopback $A.\text{accept} \xrightarrow{\epsilon} A.\text{start}$, and bypass $s \xrightarrow{\epsilon} a$ (allowing zero occurrences). Returns fragment $(s, a)$.
6. **Positive Closure ($A^+$)**: Pops $A$. Identical to Kleene closure with loopback $A.\text{accept} \xrightarrow{\epsilon} A.\text{start}$, but **without** the bypass from start to accept (forcing at least 1 match). Returns fragment $(s, a)$.

#### Combined Master NFA
Once all 18 token rule NFAs are constructed:
* A single global **Master Start State** ($S_0$) is instantiated.
* An $\epsilon$-transition is added from $S_0$ directly to the start state of every individual token's NFA.
* Each token's accept state is labeled with its token class name (`boolean`, `int`, `id`, `num`, etc.) and its priority number.
* This prepares the NFA for Step 2 (Subset Construction), where the initial DFA state is the $\epsilon$-closure of $S_0$.

### 3.6 Subset Construction: NFA $\to$ DFA (Step 2)
Implemented in [`DFABuilder`](../src/DFABuilder.cpp). Every DFA state stands for a **set of NFA states** that the NFA could be in at the same time after reading the same input.

* **$\epsilon$-closure** (`DFABuilder::epsilonClosure`): all NFA states reachable from a set $T$ using only $\epsilon$-transitions (including $T$ itself). It uses a work-stack: pop a state, push every $\epsilon$-target that is not yet in the result. Because a target is only pushed when it is *new*, $\epsilon$-cycles cannot loop forever.
* **move** (`DFABuilder::move`): $\text{move}(T, c)$ = all NFA states reachable from any state of $T$ by reading the single character $c$ (no $\epsilon$-moves yet).
* **Algorithm** (`DFABuilder::buildFromNFA`):
  1. The start DFA state is $\epsilon\text{-closure}(\{S_0\})$, where $S_0$ is the Master Start State. It receives ID 0.
  2. Keep a dictionary `map<set<int>, int>` from NFA-state-sets to DFA state IDs, and a queue of DFA states whose transitions are not computed yet.
  3. Take a DFA state $D$ from the queue. For every character $c$ in the alphabet, compute $U = \epsilon\text{-closure}(\text{move}(D, c))$.
  4. If $U$ is empty there is no transition (implicit dead state). Otherwise find or create the DFA state for $U$ (new states are queued) and record $D \xrightarrow{c} U$.
  5. Repeat until the queue is empty.
* **Priority resolution (tie-break)**: when a DFA state contains accepting NFA states of several tokens, it becomes accepting for the token with the **lowest priority number** (the rule listed first). For example, the group reached after reading `i`, `f` contains both `if` (priority 3) and `id` (priority 12), so it accepts as `if`. This is what makes keywords win over identifiers.
* **Result on the assignment rules**: the 733-state Combined NFA becomes a **185-state DFA**.

### 3.7 DFA Minimization (Step 3)
Implemented in [`DFAMinimizer`](../src/DFAMinimizer.cpp) using **partition refinement** (Moore's equivalence-partitioning algorithm; it yields the same minimal DFA as Hopcroft's algorithm and is simpler to implement).

1. **Remove unreachable states**: a BFS from the start state marks every reachable state; the rest are ignored.
2. **Initial partition**: reachable states are split into *non-accepting* states, and *accepting states grouped by token class*. **Crucial rule**: accepting states of different token classes (e.g. `id` vs `num` vs `if`) are never placed in the same initial group, otherwise the lexer would lose track of which token was matched.
3. **Refinement**: every state gets a *signature* = (its current group, the group reached on every character). A missing transition counts as a special "dead" group (`-1`). States with the same signature stay together; states with different signatures are split into new groups. This repeats until a full pass splits nothing.
4. **Build the minimal DFA**: one new state per final group, with the transitions and token class taken from any member of the group. States are numbered in **BFS order from the start state** (start state = 0, deterministic output on every run).
* **Result on the assignment rules**: the 185-state DFA shrinks to a **44-state minimal DFA** (40 accepting states, all 18 token classes preserved). Minimizing the result a second time changes nothing, which confirms it is minimal.

### 3.8 Runtime Simulation Engine (Step 4)
Implemented in [`LexicalAnalyzer`](../src/LexicalAnalyzer.cpp) and [`SymbolTable`](../src/SymbolTable.cpp):

* **Maximal Munch (Longest Prefix Matching)**:
  * The scanner reads the source code character by character from the current input cursor.
  * It follows transitions in the minimal DFA while maintaining:
    * `lastAcceptingState`: The most recent accepting DFA state reached.
    * `lastAcceptingPos`: The cursor position when that accepting state was reached.
  * When the scanner encounters a dead state (no valid transition for the current character), it **backtracks** to `lastAcceptingPos`.
  * The lexeme is extracted as `input[startPos ... lastAcceptingPos]`, the winning token class is emitted, and the cursor is reset to `lastAcceptingPos`.
  * If the token class is `"id"`, the lexeme is inserted into the `SymbolTable`.
* **Whitespace Handling**: Spaces, tabs, and newlines between tokens are skipped while updating line and column counters.
* **Error Recovery Routine**:
  * If from `startPos` no accepting state was ever visited before reaching a dead state, an error recovery routine is triggered:
    1. An informative diagnostic message is generated:
       `Lexical Error at line L, column C: Unrecognized character 'X'` (or hex formatted `\xHH` if unprintable).
    2. The illegal character is skipped (cursor advances by 1).
    3. The scanner resumes looking for subsequent tokens without aborting execution.
* **Phase 2 Parser Integration Interface**:
  * The `LexicalAnalyzer` class exposes `Token getNextToken()` and `bool hasMoreTokens()` to allow the future Phase 2 syntax analyzer / parser to consume tokens lazily on demand.

---

## 4. Section C: Resultant Transition Table for Minimal DFA

The table below is produced automatically from `tests/lexical_rules.txt` by the pipeline *RuleParser → NFABuilder → DFABuilder → DFAMinimizer*; nothing is typed by hand.

| Stage | Number of states |
|---|---|
| Combined NFA (Step 1) | 733 |
| DFA after subset construction (Step 2) | 185 |
| **Minimal DFA (Step 3)** | **44** (40 accepting) |

### How to read the table
* Each **row is a state**; the start state is **0** (marked ▶). A state marked `*` is **accepting**, and the *Accepts* column gives the token class.
* Each **number is a destination**: "on this character, move to this state". A state lists only the characters that are allowed next; every other character means *no move*. A row with several numbers therefore means several characters are allowed next, and a row with none means the token is always complete at that point (e.g. `(`, `)`, `+`, `*`).
* *Reached by* gives the shortest input that leads to the state, which shows what the state "remembers" (for example state 15 means "I have read `i`", state 24 means "I have read `if`").
* Letters and digits are grouped to keep the table readable.

Example trace for `while`: `0 -w-> 16 -h-> 26 -i-> 32 -l-> 37 -e-> 41`, and state 41 accepts as `while`.

### Minimal DFA transition table (compact form)

| State | Accepts | Reached by | Moves (characters → next state) |
|---|---|---|---|
| **▶ 0** |  | *(start)* | `!` → **1**<br>`(` → **2**<br>`)` → **3**<br>`*` `/` → **4**<br>`+` `-` → **5**<br>`,` → **6**<br>any digit → **7**<br>`;` → **8**<br>`<` `>` → **9**<br>`=` → **10**<br>any letter except b,e,f,i,w → **11**<br>`b` → **12**<br>`e` → **13**<br>`f` → **14**<br>`i` → **15**<br>`w` → **16**<br>`{` → **17**<br>`}` → **18** |
| **1** |  | `!` | `=` → **19** |
| **2\*** | `(` | `(` | *(none — token is complete)* |
| **3\*** | `)` | `)` | *(none — token is complete)* |
| **4\*** | `mulop` | `*` | *(none — token is complete)* |
| **5\*** | `addop` | `+` | *(none — token is complete)* |
| **6\*** | `,` | `,` | *(none — token is complete)* |
| **7\*** | `num` | `0` | any digit → **7**<br>`.` → **20** |
| **8\*** | `;` | `;` | *(none — token is complete)* |
| **9\*** | `relop` | `<` | `=` → **19** |
| **10\*** | `assign` | `=` | `=` → **19** |
| **11\*** | `id` | `a` | any letter, any digit → **11** |
| **12\*** | `id` | `b` | any letter except o, any digit → **11**<br>`o` → **21** |
| **13\*** | `id` | `e` | any letter except l, any digit → **11**<br>`l` → **22** |
| **14\*** | `id` | `f` | any letter except l, any digit → **11**<br>`l` → **23** |
| **15\*** | `id` | `i` | any letter except f,n, any digit → **11**<br>`f` → **24**<br>`n` → **25** |
| **16\*** | `id` | `w` | any letter except h, any digit → **11**<br>`h` → **26** |
| **17\*** | `{` | `{` | *(none — token is complete)* |
| **18\*** | `}` | `}` | *(none — token is complete)* |
| **19\*** | `relop` | `!=` | *(none — token is complete)* |
| **20** |  | `0.` | any digit → **27** |
| **21\*** | `id` | `bo` | any letter except o, any digit → **11**<br>`o` → **28** |
| **22\*** | `id` | `el` | any letter except s, any digit → **11**<br>`s` → **29** |
| **23\*** | `id` | `fl` | any letter except o, any digit → **11**<br>`o` → **30** |
| **24\*** | `if` | `if` | any letter, any digit → **11** |
| **25\*** | `id` | `in` | any letter except t, any digit → **11**<br>`t` → **31** |
| **26\*** | `id` | `wh` | any letter except i, any digit → **11**<br>`i` → **32** |
| **27\*** | `num` | `0.0` | any digit → **27**<br>`E` → **33** |
| **28\*** | `id` | `boo` | any letter except l, any digit → **11**<br>`l` → **34** |
| **29\*** | `id` | `els` | any letter except e, any digit → **11**<br>`e` → **35** |
| **30\*** | `id` | `flo` | any letter except a, any digit → **11**<br>`a` → **36** |
| **31\*** | `int` | `int` | any letter, any digit → **11** |
| **32\*** | `id` | `whi` | any letter except l, any digit → **11**<br>`l` → **37** |
| **33** |  | `0.0E` | any digit → **38** |
| **34\*** | `id` | `bool` | any letter except e, any digit → **11**<br>`e` → **39** |
| **35\*** | `else` | `else` | any letter, any digit → **11** |
| **36\*** | `id` | `floa` | any letter except t, any digit → **11**<br>`t` → **40** |
| **37\*** | `id` | `whil` | any letter except e, any digit → **11**<br>`e` → **41** |
| **38\*** | `num` | `0.0E0` | any digit → **38** |
| **39\*** | `id` | `boole` | any letter except a, any digit → **11**<br>`a` → **42** |
| **40\*** | `float` | `float` | any letter, any digit → **11** |
| **41\*** | `while` | `while` | any letter, any digit → **11** |
| **42\*** | `id` | `boolea` | any letter except n, any digit → **11**<br>`n` → **43** |
| **43\*** | `boolean` | `boolean` | any letter, any digit → **11** |

### Full matrix (all 77 characters)
The complete 44 × 77 matrix is emitted by the program in two formats (for the report and for spreadsheets):
* [`minimal_dfa_table.txt`](minimal_dfa_table.txt): aligned text, rows = states, columns = alphabet characters, `-` = no transition.
* [`minimal_dfa_table.csv`](minimal_dfa_table.csv): the same matrix as CSV (State, Start, Accepting, Token, then one column per character).

They are generated by `DFA::saveTransitionTableText` and `DFA::saveTransitionTableCSV` (see `tests/test_step3.cpp`).

---

## 5. Section D: Resultant Stream of Tokens for Example Program

### Test Program
From Page 3 of the assignment specification (saved in [`tests/sample_program.java`](../tests/sample_program.java)):
```java
int sum , count , pass , mnt; while (pass !=10)
{
pass = pass + 1 ;
}
```

### Resultant Token Stream (`output_tokens.txt`)
Emitted by `lexer_generator` (format: one recognized token class per line, matching Page 3 of the PDF):
```text
int
id
,
id
,
id
,
id
;
while
(
id
relop
num
)
{
id
assign
id
addop
num
;
}
```

### Resultant Symbol Table
After scanning the test program, the Symbol Table records the following entries:

| Identifier | Token Type | First Line | Occurrences |
|---|---|---|---|
| `sum` | `id` | 1 | 1 |
| `count` | `id` | 1 | 1 |
| `pass` | `id` | 1 | 4 |
| `mnt` | `id` | 1 | 1 |

*Note: `pass` appears in the initial declaration, in the `while` condition `(pass != 10)`, on the LHS of assignment `pass =`, and inside the expression `pass + 1`, correctly totaling 4 occurrences.*

---

## 6. Section E: Assumptions Made and Justifications

1. **Keywords are Reserved**: In the provided assignment specification, `id` is listed before `{boolean int float}`. In standard Java specifications, keywords are reserved words and can never be valid identifiers. Therefore, keywords are given top priority so they win any tie-break over `id`.
2. **Alphabet Scope**: The alphabet $\Sigma$ consists of standard ASCII characters (values 0–127).
3. **Decimal Point in Numbers**: In the specification `num: digit+ | digit+ . digits ( \L | E digits)`, the dot `.` is not escaped with `\` because Rule 7 of the specification does not designate `.` as an operator. Hence, `.` is treated as a literal decimal point character.
4. **Lambda ($\Lambda$)**: `\L` represents the empty string $\epsilon$. It is handled as a free $\epsilon$-transition during automata construction.
5. **Whitespace Handling**: Spaces, tabs, and newlines between tokens are optional and are skipped by the scanner prior to matching tokens.
6. **Implicit Dead State**: The DFA and the minimal DFA do not store an explicit dead (trap) state. A missing transition means "no move". This keeps the table small, and the scanner only needs to know that no move exists in order to stop and fall back to the last accepting state.
7. **Tie-Break by Rule Order**: When several rules accept the same longest prefix, the rule listed first (lowest priority number) wins, as required by the assignment. Keywords and punctuation come before `id`, `num` and the operators.
8. **Token Classes Are Never Merged**: During minimization, accepting states of different token classes are kept in separate groups, even when their outgoing transitions look identical.
9. **Table Format**: The assignment lets us choose the output format of the transition table, so the minimal DFA is emitted as an aligned text matrix and as a CSV file (rows = states, columns = alphabet characters).

---
