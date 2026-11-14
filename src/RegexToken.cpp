#include "../include/RegexToken.h"
#include <sstream>

using namespace std;

string RegexToken::toString() const {
    switch (type) {
        case TokenType::LITERAL: {
            if (value == ' ') return "' '";
            return string(1, value);
        }
        case TokenType::LAMBDA: return "\\L";
        case TokenType::CONCAT: return ".";
        case TokenType::UNION:  return "|";
        case TokenType::STAR:   return "*";
        case TokenType::PLUS:   return "+";
        case TokenType::LPAREN: return "(";
        case TokenType::RPAREN: return ")";
    }
    return "?";
}

string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LITERAL: return "LITERAL";
        case TokenType::LAMBDA:  return "LAMBDA";
        case TokenType::CONCAT:  return "CONCAT";
        case TokenType::UNION:   return "UNION";
        case TokenType::STAR:    return "STAR";
        case TokenType::PLUS:    return "PLUS";
        case TokenType::LPAREN:  return "LPAREN";
        case TokenType::RPAREN:  return "RPAREN";
    }
    return "UNKNOWN";
}

string tokensToString(const vector<RegexToken>& tokens) {
    ostringstream oss;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) oss << " ";
        oss << tokens[i].toString();
    }
    return oss.str();
}
