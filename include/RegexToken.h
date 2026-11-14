#pragma once

#include <string>
#include <vector>

using namespace std;

// The types of tokens that can exist inside a regular expression
enum class TokenType {
    LITERAL,    // Actual character to match in input: 'a', '0', '+', '=', etc.
    LAMBDA,     // \L (empty string / epsilon)
    CONCAT,     // Explicit concatenation operator (internal '.')
    UNION,      // |
    STAR,       // * (0 or more occurrences)
    PLUS,       // + (1 or more occurrences)
    LPAREN,     // (
    RPAREN      // )
};

// Represents a single token in a regular expression
struct RegexToken {
    TokenType type;
    char value; // Used only when type is LITERAL

    RegexToken(TokenType tokenType = TokenType::LITERAL, char charValue = '\0')
        : type(tokenType), value(charValue) {}

    bool operator==(const RegexToken& other) const {
        return type == other.type && value == other.value;
    }

    string toString() const;
};

// Helper printing functions for debugging and reporting
string tokenTypeToString(TokenType type);
string tokensToString(const vector<RegexToken>& tokens);
