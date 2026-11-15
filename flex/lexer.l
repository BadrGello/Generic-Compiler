%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple Symbol Table Node */
typedef struct SymbolNode {
    char name[128];
    int occurrences;
    struct SymbolNode* next;
} SymbolNode;

SymbolNode* symbolTable = NULL;

void insertSymbol(const char* name) {
    SymbolNode* curr = symbolTable;
    while (curr != NULL) {
        if (strcmp(curr->name, name) == 0) {
            curr->occurrences++;
            return;
        }
        curr = curr->next;
    }
    SymbolNode* newNode = (SymbolNode*)malloc(sizeof(SymbolNode));
    strncpy(newNode->name, name, sizeof(newNode->name) - 1);
    newNode->name[sizeof(newNode->name) - 1] = '\0';
    newNode->occurrences = 1;
    newNode->next = symbolTable;
    symbolTable = newNode;
}

void printSymbolTable() {
    printf("\n========================================================\n");
    printf("               FLEX SYMBOL TABLE (Identifiers)          \n");
    printf("========================================================\n");
    printf("%-20s %-12s\n", "Identifier", "Occurrences");
    printf("--------------------------------------------------------\n");
    SymbolNode* curr = symbolTable;
    while (curr != NULL) {
        printf("%-20s %-12d\n", curr->name, curr->occurrences);
        curr = curr->next;
    }
    printf("========================================================\n");
}
%}

%option noyywrap

/* Regular Definitions */
letter      [a-zA-Z]
digit       [0-9]
digits      {digit}+
id          {letter}({letter}|{digit})*
num         {digit}+|{digit}+\.{digits}(\L|E{digits})?

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

    /* Whitespace & Comments (skipped) */
[ \t\r\n]+  { /* ignore whitespace */ }

    /* Error Handling */
.           { fprintf(stderr, "Lexical Error: Unrecognized character '%s'\n", yytext); }

%%

int main(int argc, char** argv) {
    if (argc > 1) {
        FILE* file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
            return 1;
        }
        yyin = file;
    }
    yylex();
    printSymbolTable();
    return 0;
}
