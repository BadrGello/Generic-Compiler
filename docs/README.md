# Lexical Analyzer Generator (Phase 1)

This project is a Lexical Analyzer Generator built in C++ for the Compiler course (Alexandria University). It reads a lexical specification file, builds a minimal DFA, and simulates it to tokenize input source code.

---

## The Plan

### 0) Read Lexical Rules from Text File and Parse It
* **Input Rules format**:
  * Regular definitions: `LHS = RHS` (e.g. `letter = a-z | A-Z`)
  * Regular expressions: `LHS: RHS` (e.g. `id: letter (letter|digit)*`)
  * Keywords: enclosed in `{ }` on separate lines (e.g. `{boolean int float}`)
  * Punctuations: enclosed in `[ ]` on separate lines (e.g. `[; , \( \) { }]`)
  * `\L` represents Lambda ($\epsilon$).
  * Escaped characters: reserved symbols preceded by `\` (e.g. `\+`, `\-`, `\=\=`, `\(`).
* **Missing Details & What to do**:
  * **Don't use a simple unordered hash table for token rules**: Store rules in an ordered list (`vector`) or attach an explicit priority number (line index) to each rule. Rule order determines tie-breaking priority.
  * **Macro Expansion**: Substitute regular definitions into subsequent expressions before processing them (e.g. replace `digit` with `0-9`).
  * **Character Range Expansion**: Expand ranges like `a-z` and `0 - 9` into unions `(a|b|...|z)`.
  * **Insert Explicit Concatenation**: Regexes use juxtaposition for concatenation (e.g. `a b`). Insert an explicit operator (e.g. `.`) between adjacent symbols.
  * **Infix to Postfix**: Convert regular expressions to postfix notation (using Shunting-Yard algorithm) so Thompson's construction is simple to implement.
* **Key Assumptions**:
  * **Keywords are reserved**: Keywords have higher priority than `id`, even if `id` is written above them in the rules file.

---

### 1) RE -> NFA (Thompson's Construction)
* Construct an NFA for each token rule using standard Thompson's construction:
  * Base symbol
  * Lambda ($\epsilon$) transition
  * Concatenation (`.`)
  * Union (`|`)
  * Kleene Star (`*`)
  * Positive Closure (`+`)
* **Missing Details & What to do**:
  * **Combined NFA**: Create one single Master NFA by adding a new global start state with $\Lambda$ ($\epsilon$) transitions to the start state of every individual token's NFA.
  * **Accepting States**: Every accepting state must store its token name (e.g., `id`, `num`, `relop`, `int`) and its rule priority.

---

### 2) NFA -> DFA (Subset Construction)
* Convert the Combined NFA to a DFA using Subset Construction:
  * Compute $\epsilon\text{-closure}$ of states.
  * Compute `move(T, char)` transitions.
* **Missing Details & What to do**:
  * If a DFA state contains accepting states from multiple tokens, assign it the token with the **highest priority** (the one that appeared earliest in the rules file / keyword).

---

### 3) Minimize DFA
* **Algorithm**: Use **Hopcroft's Algorithm** or **Equivalence Partitioning**.
* **Critical Rule**:
  * Do **NOT** start by putting all accepting states into one group!
  * Accepting states of **different token classes** must start in separate initial partitions (e.g., partition for `id`, partition for `num`, partition for `relop`, partition for non-accepting states). Otherwise, different tokens will be merged together.
* Produce the final reduced transition table.

---

### 4) Simulate the Minimized DFA (Pattern Matching & Symbol Table)
* Read input source code character by character.
* **Maximal Match (Longest Prefix)**: Keep scanning characters until no transition is possible. If an accepting state was visited, backtrack to the last visited accepting state and emit that token.
* **Priority**: If two rules match the same length, pick the one with higher priority (first in rules file).
* **Blanks & Whitespace**: Skip spaces, tabs, and newlines between tokens.
* **Create a Symbol Table**: Store identifiers (`id`) and their attributes.
* **Token Output**: Output token class (and attribute values).
* **Error Recovery**: If no prefix matches, print an error message with line/character info, skip the bad character, and continue scanning.
* **Phase 2 Integration**: Wrap the simulator into a class with a `getNextToken()` function so Phase 2's parser can call it directly.

---

### 5) Testing & Output Files
* **Transition Table**: Output the minimal DFA transition table in a clear format (CSV, formatted text, or matrix).
* **Token Stream**: Output recognized token classes one per line (matches the assignment requirement):
  ```text
  int
  id
  ,
  id
  ;
  while
  ...
  ```
* Test using the provided sample code:
  ```java
  int sum , count , pass , mnt; while (pass !=10)
  {
  pass = pass + 1 ;
  }
  ```

---

### 6) Bonus & Final Deliverables
* **Bonus**: Generate a lexical analyzer for the same rules using **Flex (or Lex)**, and prepare a report with steps and screenshots.
* **Project Report**:
  1. Description of data structures used.
  2. Explanation of algorithms.
  3. The final minimal DFA transition table.
  4. The output stream of tokens for the test program.
  5. Assumptions made and justifications.

---

## Suggested Workflow & Team Division (4 People)

To avoid bottlenecks, everyone can work at the same time by agreeing on clear data interfaces and using mock data.

```
[Person 1: Rules & Regex]
       │  (Output: Postfix regex + Token Priority)
       ▼
[Person 2: NFA Engine]
       │  (Output: Combined NFA graph)
       ▼
[Person 3: DFA & Minimization]
       │  (Output: Minimized Transition Table)
       ▼
[Person 4: Lexer Engine & Symbol Table]
```

### Roles & Responsibilities

| Person | Module | Main Tasks | Output Interface |
|---|---|---|---|
| **Person 1** | **Rules Parser & Regex Engine** | - Read rules text file<br>- Expand definitions<br>- Insert concatenation operators<br>- Convert regex to postfix (Shunting-Yard) | A list of rules: `struct TokenRule { string name; int priority; string postfix_regex; }` |
| **Person 2** | **NFA Builder** | - Implement Thompson's construction operators<br>- Build sub-NFAs for each token<br>- Build Combined Master NFA with $\epsilon$ transitions | `CombinedNFA` object: state list, start state, transitions, and accepting states map |
| **Person 3** | **DFA & Minimizer** | - Implement $\epsilon$-closure and `move`<br>- Subset construction (NFA $\to$ DFA)<br>- Priority resolution for accepting states<br>- DFA Minimization (Hopcroft / Partitioning)<br>- Table exporter | `TransitionTable` class: `getNextState(state, char)`, `getAcceptingToken(state)` and table file |
| **Person 4** | **Lexer Simulator & Symbol Table** | - Implement Symbol Table<br>- Maximal munch scanning + backtracking<br>- Whitespace skipping & error recovery<br>- Callable `getNextToken()` for Phase 2<br>- Bonus (Flex tool report) | Lexer program, output token file, and Flex bonus report |

---

### How to Work in Parallel (No Waiting!)

* **Person 4 doesn't wait for Persons 1, 2, or 3**: Hardcode a small 2-state or 3-state transition table (e.g. recognizing `if` and `id`) in code and write the entire simulator, backtracking, and symbol table on Day 1.
* **Person 3 doesn't wait for Persons 1 or 2**: Draw a tiny 3-state NFA on paper, hardcode it into the `NFA` struct, and implement and verify Subset Construction and Minimization.
* **Person 2 doesn't wait for Person 1**: Test Thompson's construction on hardcoded postfix strings like `ab.c|*`.
* **Person 1**: Focuses on solid parsing of definitions, ranges, escape characters, and Shunting-Yard.

---

### Project Milestones

1. **Milestone 1 (Interfaces & Setup)**: Agree on C++ structs (`State`, `NFA`, `DFA`, `Token`, `TransitionTable`).
2. **Milestone 2 (Unit Components Ready)**:
   - Parser outputs clean postfix strings.
   - NFA builder handles all regex operators.
   - DFA converter + minimizer produces reduced tables.
   - Simulator runs correctly on a mock table.
3. **Milestone 3 (Pipeline Integration)**: Connect 1 $\to$ 2 $\to$ 3 $\to$ 4 into one unified program.
4. **Milestone 4 (Verification & Report)**:
   - Run the provided Java subset test program.
   - Run Flex for the bonus task.
   - Compile the PDF report with tables and screenshots.
