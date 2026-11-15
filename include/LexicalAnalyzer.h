#pragma once

#include "DFA.h"
#include "Token.h"
#include "SymbolTable.h"
#include <string>
#include <vector>
#include <iostream>

using namespace std;

// Lexical Analyzer runtime simulator using Maximal Munch (longest-prefix) matching
class LexicalAnalyzer {
private:
    DFA dfa;
    string input;
    size_t cursor = 0;
    int currentLine = 1;
    int currentColumn = 1;

    SymbolTable symbolTable;
    vector<string> errorLog;
    ostream* errorStream = &cerr;

    void skipWhitespace();
    void recoverFromError(char badChar, int line, int col);

public:
    explicit LexicalAnalyzer(const DFA& minimizedDfa, const string& sourceCode = "", ostream* errStream = &cerr);

    Token getNextToken();

    // Returns true if there are more non-whitespace characters to scan before EOF
    bool hasMoreTokens();

    // Tokenizes all remaining input until EOF
    vector<Token> tokenizeAll();

    // Accessors
    const SymbolTable& getSymbolTable() const { return symbolTable; }
    SymbolTable& getSymbolTable() { return symbolTable; }
    const vector<string>& getErrors() const { return errorLog; }
    bool hasErrors() const { return !errorLog.empty(); }

    // Resets scanner with new source code
    void setSourceCode(const string& sourceCode);
};
