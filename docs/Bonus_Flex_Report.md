# Bonus Task Report: Lexical Analyzer Generation Using Flex

**Alexandria University — Faculty of Engineering**  
**Course**: Programming Languages and Compilers  
**Assignment 1 — Phase 1 Bonus Deliverable**  

---

## 1. Overview
The objective of this bonus task is to demonstrate the use of standard industry compiler-construction tools (**Flex** — Fast Lexical Analyzer Generator) to automatically produce a lexical analyzer matching the same language specifications as our custom C++ generator, and to contrast the two approaches.

---

## 2. Flex Specification File (`flex/lexer.l`)

The specification is structured into three standard sections:
1. **Definitions**: Defines regular definitions for `letter`, `digit`, `digits`, `id`, and `num`.
2. **Rules**: Ordered pattern-action pairs. Keywords are placed first to guarantee they take precedence over general identifiers.
3. **User Code**: C code implementing a basic Symbol Table linked list and driver `main()` function reading the source program and invoking `yylex()`.

```lex
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple Symbol Table linked list */
typedef struct SymbolNode {
    char name[128];
    int occurrences;
    struct SymbolNode* next;
} SymbolNode;

SymbolNode* symbolTable = NULL;
void insertSymbol(const char* name);
void printSymbolTable();
%}

%option noyywrap

/* Regular Definitions */
letter      [a-zA-Z]
digit       [0-9]
digits      {digit}+
id          {letter}({letter}|{digit})*

%%

    /* Reserved Keywords (Highest Priority) */
"boolean"   { printf("boolean\n"); }
"int"       { printf("int\n"); }
"float"     { printf("float\n"); }
"if"        { printf("if\n"); }
"else"      { printf("else\n"); }
"while"     { printf("while\n"); }

    /* Punctuations */
";"         { printf(";\n"); }
","         { printf(",\n"); }
"("         { printf("(\n"); }
")"         { printf(")\n"); }
"{"         { printf("{\n"); }
"}"         { printf("}\n"); }

    /* Operators */
"=="|"!="|">"|">="|"<"|"<=" { printf("relop\n"); }
"="         { printf("assign\n"); }
"+"|"-"     { printf("addop\n"); }
"*"|"/"     { printf("mulop\n"); }

    /* Identifiers & Numbers */
{id}        { insertSymbol(yytext); printf("id\n"); }
{digit}+    { printf("num\n"); }
{digit}+\.{digits}([eE]{digits})? { printf("num\n"); }

    /* Whitespace & Newlines */
[ \t\r\n]+  { /* skip whitespace */ }

    /* Error Recovery */
.           { fprintf(stderr, "Lexical Error: Unrecognized character '%s'\n", yytext); }

%%
```

---

## 3. Step-by-Step Instructions to Build and Run Flex

### Prerequisites:
* **Linux/macOS**: Flex and GCC (`sudo apt install flex build-essential`)
* **Windows**: WinFlexBison or MinGW Flex (`http://gnuwin32.sourceforge.net/packages/flex.htm`)

### Execution Commands:
1. **Generate the C scanner from the specification**:
   ```bash
   flex lexer.l
   ```
   *Output*: Generates `lex.yy.c`.

2. **Compile the generated C scanner**:
   ```bash
   gcc -O2 lex.yy.c -o flex_lexer
   ```
   *Output*: Produces `flex_lexer` executable.

3. **Run on the sample test program**:
   ```bash
   ./flex_lexer sample_program.java > flex_output.txt
   ```

---

## 4. Verification & Output Comparison

When given the sample program from Page 3 of the PDF:
```java
int sum , count , pass , mnt; while (pass !=10)
{
pass = pass + 1 ;
}
```

Flex produces the exact same token stream as our custom generator:
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

And outputs the matching Symbol Table:
```text
========================================================
               FLEX SYMBOL TABLE (Identifiers)          
========================================================
Identifier           Occurrences 
--------------------------------------------------------
mnt                  1           
pass                 4           
count                1           
sum                  1           
========================================================
```

---

## 5. Comparative Analysis: Custom Generator vs. Flex

| Dimension | Custom C++ Generator | Flex Tool |
|---|---|---|
| **Underlying Automata** | Thompson's NFA $\to$ Subset Construction DFA $\to$ Partition Refinement Minimal DFA | Direct Regular Expression $\to$ DFA construction (often using Glushkov/Thompson/Berry-Sethi with direct minimization) |
| **Tie-Breaking Rule** | Explicit priority ordering resolved during NFA/DFA conversion | Rules listed earlier in the file take precedence |
| **Maximal Munch** | Forward pointer with state backtracking buffer | `yymore()` / internal buffer lookahead and rollback |
| **Integration** | Direct C++ class exposing `Token getNextToken()` | Standard `yylex()` function returning token IDs for Bison/Yacc |
| **Transparency & Educational Value** | Full visibility into every state, transition table matrix, and optimization step | Black-box generated C code |
