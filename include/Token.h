#pragma once

#include <string>
#include <iostream>

using namespace std;

// Represents a recognized lexical token
struct Token {
    string tokenClass;     // e.g. "int", "id", "num", ";", "relop", "EOF"
    string attributeValue; // Matched lexeme value (e.g. "sum", "10", "!=")
    int line = 1;
    int column = 1;

    Token() : tokenClass(""), attributeValue(""), line(1), column(1) {}
    Token(const string& tClass, const string& attr, int l = 1, int c = 1)
        : tokenClass(tClass), attributeValue(attr), line(l), column(c) {}

    bool isEOF() const { return tokenClass == "EOF"; }

    friend ostream& operator<<(ostream& os, const Token& t) {
        os << "Token(" << t.tokenClass << ", \"" << t.attributeValue << "\", line=" << t.line << ", col=" << t.column << ")";
        return os;
    }
};
