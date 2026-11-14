# Phase 1 - Step 0: Lexical Rules Parser & Regex Preprocessor

## 1. Overview
The goal of Step 0 is to read the lexical specification file (`lexical_rules.txt`), categorize each rule, expand definition macros and character ranges, insert explicit concatenation operators, and transform all regular expressions from standard infix notation to **Postfix (Reverse Polish Notation)** using Dijkstra's Shunting-Yard algorithm.

The resulting output provides a clean, prioritized list of token specifications ready for Thompson's NFA construction in Step 1.

---

## 2. Data Structures Used

### 2.1 `TokenType` Enum
Defines the atomic token types recognized within regular expressions:
```cpp
enum class TokenType {
    LITERAL,    // Concrete character to match (e.g. 'a', '0', '+', '=')
    LAMBDA,     // \L (empty string / epsilon)
    CONCAT,     // Explicit concatenation operator (internal)
    UNION,      // |
    STAR,       // * (Kleene closure: 0 or more)
    PLUS,       // + (Positive closure: 1 or more)
    LPAREN,     // (
    RPAREN      // )
};
```

### 2.2 `RegexToken` Struct
Represents an individual token in the expression stream:
* `TokenType type`: The type of the token.
* `char value`: The ASCII character value (meaningful when `type == TokenType::LITERAL`).

### 2.3 `TokenRule` Struct
Encapsulates a finalized token definition:
* `std::string name`: Token name (e.g., `id`, `num`, `relop`, `int`, `;`).
* `int priority`: Integer representing tie-breaking precedence (lower number = higher priority).
* `std::string rawRegex`: Original regular expression string from file.
* `std::string expandedRegex`: String after macro and range expansions.
* `std::vector<RegexToken> infix`: Infix tokens with explicit concatenation inserted.
* `std::vector<RegexToken> postfix`: Final postfix token stream for Thompson's construction.

---

## 3. Algorithms & Techniques

### 3.1 Line Categorization & Ordering
The input file contains four distinct line formats:
1. **Regular Definitions** (`LHS = RHS`): Stored in a definition table and expanded sequentially.
2. **Regular Expressions** (`LHS: RHS`): Given sequential priorities starting after keywords and punctuations.
3. **Keywords** (`{ word1 word2 ... }`): Reserved words; extracted and given the **highest priority** (priority 0 to $K-1$) so they are never misidentified as identifiers.
4. **Punctuations** (`[ sym1 sym2 ... ]`): Extracted, unescaped, and given priority immediately following keywords.

### 3.2 Macro Definition Expansion
When a regular expression or definition uses a previously defined identifier (e.g. `digits = digit+` uses `digit`, and `id` uses `letter` and `digit`):
* Definitions are sorted by descending length to prevent partial substring collisions (e.g. `digits` vs `digit`).
* Full-word boundaries (`[^a-zA-Z0-9_]`) are verified before replacing the identifier with `(RHS)`.

### 3.3 Character Range Expansion
Patterns like `a-z`, `A-Z`, and `0 - 9` (accounting for optional whitespace around `-`) are detected:
* Checked to ensure `-` is not an escaped literal `\-`.
* Expanded into explicit unions: `(a|b|c|...|z)` and `(0|1|...|9)`.
* This simplifies Thompson's construction by reducing character ranges to standard single-character unions.

### 3.4 Escape Character & Lambda Handling
* `\L` is recognized as the empty string $\Lambda$ ($\epsilon$).
* Characters preceded by backslash (`\+`, `\-`, `\*`, `\/`, `\=\=`, `\!\=`, `\(`, `\)`) have their backslash stripped and are emitted with `TokenType::LITERAL`. This ensures the lexer does not treat literal `+` or `*` as regex repeat operators.

### 3.5 Explicit Concatenation Insertion
In standard regular expressions, concatenation is implicit (e.g. `a b` or `letter (letter|digit)*`).
Before conversion to postfix, an explicit `CONCAT` token is inserted between any adjacent pair of tokens where:
* `prev` $\in$ `{LITERAL, LAMBDA, RPAREN, STAR, PLUS}`
* `curr` $\in$ `{LITERAL, LAMBDA, LPAREN}`

### 3.6 Infix to Postfix Conversion (Shunting-Yard)
Operator precedence table:
| Operator | Type | Precedence | Associativity |
|---|---|---|---|
| `*`, `+` | Unary Postfix | 3 | N/A (applied immediately) |
| `CONCAT` | Binary Infix | 2 | Left-to-Right |
| `UNION` (`\|`) | Binary Infix | 1 | Left-to-Right |
| `(`, `)` | Grouping | 0 | N/A |

Unary postfix operators (`*`, `+`) are emitted directly to the postfix output since they bind to the immediate preceding operand. Binary operators (`CONCAT`, `UNION`) and parentheses are managed via Dijkstra's Shunting-Yard operator stack.

---

## 4. Assumptions & Justifications

1. **Keywords are Reserved**: In the sample input, `id: letter (letter|digit)*` is listed before `{boolean int float}`. However, because keywords are reserved words in Java, they must take precedence over user-defined identifiers. Hence, keywords are assigned top priority (0 to $K-1$).
2. **Alphabet Scope**: The scanner supports standard printable ASCII characters (0–127).
3. **Escapes in Definitions**: Any symbol listed in Rule 7 (`-`, `|`, `+`, `*`, `(`, `)`) is treated as an operator unless preceded by `\`. Characters outside this set (such as the decimal dot `.` in `num`) are treated as literal characters.
4. **Lambda Representation**: `\L` is preserved as an explicit epsilon token to be handled by $\epsilon$-transitions in Thompson's NFA construction.

---

## 5. Verification & Test Results
The test suite in `tests/test_step0.cpp` validates all rules from the assignment specification:
* **Keywords**: Verified priorities 0 to 5 (`boolean`, `int`, `float`, `if`, `else`, `while`).
* **Punctuations**: Verified extraction of `;`, `,`, `(`, `)`, `{`, `}`.
* **Macro Expansion**: Verified `id` expands `letter` and `digit` and terminates with a `CONCAT` operator in postfix.
* **Numbers**: Verified `num` contains literal `.` and `\L`.
* **Operators**: Verified `relop` handles `\=\=`, `!\=`, `>`, `>\=`, `<`, `<\=`, and `addop`/`mulop` handle `\+`, `\-`, `\*`, `/`.
* **Unit Tests**: Verified correct Shunting-Yard operator precedence (`a (b|c)*` $\to$ `a b c | * .`).
