#include "../include/RuleParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <stack>
#include <algorithm>
#include <cctype>

using namespace std;

// Trims leading and trailing whitespace characters from a string
string RuleParser::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Reads and parses a lexical rules file from disk
vector<TokenRule> RuleParser::parseFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error: Could not open rule file: " << filePath << endl;
        return {};
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return parseContent(buffer.str());
}

// Parses lexical rules from an in-memory string content
vector<TokenRule> RuleParser::parseContent(const string& content) {
    definitions.clear();
    definitionOrder.clear();
    vector<TokenRule> rules;
    int nextPriority = 0;

    stringstream ss(content);
    string line;
    vector<string> rawLines;

    while (getline(ss, line)) {
        string trimmed = trim(line);
        if (trimmed.empty()) continue;
        rawLines.push_back(trimmed);
    }

    // Separate lines into their corresponding categories
    vector<string> keywordLines;
    vector<string> punctLines;
    vector<string> expressionLines;

    for (const auto& l : rawLines) {
        if (l.front() == '{' && l.back() == '}') {
            keywordLines.push_back(l);
        } else if (l.front() == '[' && l.back() == ']') {
            punctLines.push_back(l);
        } else {
            // Distinguish definitions (LHS = RHS) from expressions (LHS: RHS)
            size_t colonPos = l.find(':');
            size_t eqPos = l.find('=');

            if (colonPos != string::npos && (eqPos == string::npos || colonPos < eqPos)) {
                expressionLines.push_back(l);
            } else if (eqPos != string::npos) {
                processDefinitionLine(l);
            }
        }
    }

    // 1. Process Keywords (Reserved -> highest priority 0 .. K-1)
    for (const auto& l : keywordLines) {
        processKeywordLine(l, rules, nextPriority);
    }

    // 2. Process Punctuations (Next priority group)
    for (const auto& l : punctLines) {
        processPunctuationLine(l, rules, nextPriority);
    }

    // 3. Process Regular Expressions in their appearance order
    for (const auto& l : expressionLines) {
        processExpressionLine(l, rules, nextPriority);
    }

    return rules;
}

// Handles keyword lines like: {boolean int float} or { if else while }
void RuleParser::processKeywordLine(const string& line, vector<TokenRule>& rules, int& nextPriority) {
    string inner = line.substr(1, line.length() - 2);
    stringstream ss(inner);
    string word;

    while (ss >> word) {
        TokenRule rule(word, nextPriority++, word);
        rule.expandedRegex = word;

        // Each character in the keyword is a literal to match in order
        for (char c : word) {
            rule.infix.push_back(RegexToken(TokenType::LITERAL, c));
        }
        rule.infix = insertConcatenation(rule.infix);
        rule.postfix = infixToPostfix(rule.infix);
        rules.push_back(rule);
    }
}

// Handles punctuation lines like: [; , \( \) { }]
void RuleParser::processPunctuationLine(const string& line, vector<TokenRule>& rules, int& nextPriority) {
    string inner = line.substr(1, line.length() - 2);
    stringstream ss(inner);
    string token;

    while (ss >> token) {
        // Normalize Greek question mark if copied from certain PDFs
        if (token == "\xCD\xBE" || token == "\xCE\xBE") {
            token = ";";
        }

        // Strip backslash escape if present (e.g. \( -> ()
        string literalStr;
        for (size_t i = 0; i < token.length(); ++i) {
            if (token[i] == '\\' && i + 1 < token.length()) {
                literalStr += token[i + 1];
                i++;
            } else {
                literalStr += token[i];
            }
        }

        TokenRule rule(literalStr, nextPriority++, literalStr);
        rule.expandedRegex = literalStr;

        for (char c : literalStr) {
            rule.infix.push_back(RegexToken(TokenType::LITERAL, c));
        }
        rule.infix = insertConcatenation(rule.infix);
        rule.postfix = infixToPostfix(rule.infix);
        rules.push_back(rule);
    }
}

// Handles regular definitions like: letter = a-z | A-Z
void RuleParser::processDefinitionLine(const string& line) {
    size_t eqPos = line.find('=');
    string lhs = trim(line.substr(0, eqPos));
    string rhs = trim(line.substr(eqPos + 1));

    // Expand previously defined definitions inside this new definition
    string expandedRhs = expandDefinitions(rhs);
    // Expand ranges (e.g. 0 - 9 -> (0|1|...|9))
    expandedRhs = expandRanges(expandedRhs);

    definitions[lhs] = expandedRhs;
    definitionOrder.push_back({lhs, expandedRhs});
}

// Handles regular expressions like: id: letter (letter|digit)*
void RuleParser::processExpressionLine(const string& line, vector<TokenRule>& rules, int& nextPriority) {
    size_t colonPos = line.find(':');
    string lhs = trim(line.substr(0, colonPos));
    string rhs = trim(line.substr(colonPos + 1));

    TokenRule rule = buildTokenRule(lhs, nextPriority++, rhs);
    rules.push_back(rule);
}

// Replaces definition names with their expanded parenthesized bodies
string RuleParser::expandDefinitions(const string& str) {
    // Sort definitions by descending length to prevent partial word collisions (e.g. "digits" before "digit")
    vector<pair<string, string>> sortedDefs = definitionOrder;
    sort(sortedDefs.begin(), sortedDefs.end(), [](const auto& a, const auto& b) {
        return a.first.length() > b.first.length();
    });

    string result = str;
    for (const auto& def : sortedDefs) {
        const string& key = def.first;
        const string& val = "(" + def.second + ")";

        size_t pos = 0;
        while ((pos = result.find(key, pos)) != string::npos) {
            // Ensure whole-word matching (not a substring of a larger identifier)
            bool boundaryBefore = (pos == 0 || (!isalnum(result[pos - 1]) && result[pos - 1] != '_'));
            bool boundaryAfter = (pos + key.length() >= result.length() ||
                                  (!isalnum(result[pos + key.length()]) && result[pos + key.length()] != '_'));

            if (boundaryBefore && boundaryAfter) {
                result.replace(pos, key.length(), val);
                pos += val.length();
            } else {
                pos += key.length();
            }
        }
    }
    return result;
}

// Expands character ranges like a-z or 0 - 9 into unions (a|b|...|z)
string RuleParser::expandRanges(const string& str) {
    string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (isalnum(str[i])) {
            char startChar = str[i];
            size_t j = i + 1;
            while (j < str.length() && (str[j] == ' ' || str[j] == '\t')) {
                j++;
            }
            if (j < str.length() && str[j] == '-') {
                size_t k = j + 1;
                while (k < str.length() && (str[k] == ' ' || str[k] == '\t')) {
                    k++;
                }
                if (k < str.length() && isalnum(str[k])) {
                    char endChar = str[k];
                    bool sameType = (islower(startChar) && islower(endChar)) ||
                                    (isupper(startChar) && isupper(endChar)) ||
                                    (isdigit(startChar) && isdigit(endChar));

                    if (sameType && startChar <= endChar) {
                        // Expand to (c1|c2|...|cn)
                        result += "(";
                        for (char c = startChar; c <= endChar; ++c) {
                            result += c;
                            if (c < endChar) result += "|";
                        }
                        result += ")";
                        i = k;
                        continue;
                    }
                }
            }
        }
        result += str[i];
    }
    return result;
}

// Converts a regex string into an internal list of RegexTokens
vector<RegexToken> RuleParser::tokenizeRegex(const string& regexStr) {
    vector<RegexToken> tokens;

    for (size_t i = 0; i < regexStr.length(); ++i) {
        char c = regexStr[i];

        if (isspace(c)) {
            continue; // Ignore formatting spaces
        }

        if (c == '\\') {
            if (i + 1 < regexStr.length()) {
                char nextC = regexStr[i + 1];
                if (nextC == 'L') {
                    tokens.push_back(RegexToken(TokenType::LAMBDA));
                    i++;
                } else {
                    // Escaped literal (e.g. \+, \-, \=, \!, \(, \))
                    tokens.push_back(RegexToken(TokenType::LITERAL, nextC));
                    i++;
                }
            } else {
                tokens.push_back(RegexToken(TokenType::LITERAL, '\\'));
            }
        } else if (c == '|') {
            tokens.push_back(RegexToken(TokenType::UNION));
        } else if (c == '*') {
            tokens.push_back(RegexToken(TokenType::STAR));
        } else if (c == '+') {
            tokens.push_back(RegexToken(TokenType::PLUS));
        } else if (c == '(') {
            tokens.push_back(RegexToken(TokenType::LPAREN));
        } else if (c == ')') {
            tokens.push_back(RegexToken(TokenType::RPAREN));
        } else {
            // Ordinary character (e.g. 'a', '0', '.', '=', '>', etc.)
            tokens.push_back(RegexToken(TokenType::LITERAL, c));
        }
    }

    return tokens;
}

// Inserts explicit CONCAT operators between adjacent operands
vector<RegexToken> RuleParser::insertConcatenation(const vector<RegexToken>& tokens) {
    if (tokens.empty()) return {};

    vector<RegexToken> result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        result.push_back(tokens[i]);

        if (i + 1 < tokens.size()) {
            const RegexToken& prev = tokens[i];
            const RegexToken& curr = tokens[i + 1];

            bool prevCanEnd = (prev.type == TokenType::LITERAL ||
                               prev.type == TokenType::LAMBDA ||
                               prev.type == TokenType::RPAREN ||
                               prev.type == TokenType::STAR ||
                               prev.type == TokenType::PLUS);

            bool currCanStart = (curr.type == TokenType::LITERAL ||
                                 curr.type == TokenType::LAMBDA ||
                                 curr.type == TokenType::LPAREN);

            if (prevCanEnd && currCanStart) {
                result.push_back(RegexToken(TokenType::CONCAT));
            }
        }
    }

    return result;
}

// Converts an infix token stream to postfix using Dijkstra's Shunting-Yard algorithm
vector<RegexToken> RuleParser::infixToPostfix(const vector<RegexToken>& infixTokens) {
    vector<RegexToken> postfix;
    stack<RegexToken> opStack;

    auto getPrecedence = [](TokenType type) -> int {
        switch (type) {
            case TokenType::STAR:
            case TokenType::PLUS:   return 3;
            case TokenType::CONCAT: return 2;
            case TokenType::UNION:  return 1;
            default:                return 0;
        }
    };

    for (const auto& token : infixTokens) {
        switch (token.type) {
            case TokenType::LITERAL:
            case TokenType::LAMBDA:
                postfix.push_back(token);
                break;

            case TokenType::LPAREN:
                opStack.push(token);
                break;

            case TokenType::RPAREN:
                while (!opStack.empty() && opStack.top().type != TokenType::LPAREN) {
                    postfix.push_back(opStack.top());
                    opStack.pop();
                }
                if (!opStack.empty() && opStack.top().type == TokenType::LPAREN) {
                    opStack.pop(); // Pop matching LPAREN
                }
                break;

            case TokenType::STAR:
            case TokenType::PLUS:
                // Unary postfix operators apply immediately to the preceding operand
                postfix.push_back(token);
                break;

            case TokenType::CONCAT:
            case TokenType::UNION: {
                int currPrec = getPrecedence(token.type);
                // Pop operators with greater or equal precedence
                while (!opStack.empty() && opStack.top().type != TokenType::LPAREN &&
                       getPrecedence(opStack.top().type) >= currPrec) {
                    postfix.push_back(opStack.top());
                    opStack.pop();
                }
                opStack.push(token);
                break;
            }
        }
    }

    // Pop any remaining operators
    while (!opStack.empty()) {
        if (opStack.top().type != TokenType::LPAREN) {
            postfix.push_back(opStack.top());
        }
        opStack.pop();
    }

    return postfix;
}

// Builds a complete TokenRule from a rule name and raw regular expression
TokenRule RuleParser::buildTokenRule(const string& name, int priority, const string& rawExpr) {
    TokenRule rule(name, priority, rawExpr);

    // 1. Expand macro definitions
    string expanded = expandDefinitions(rawExpr);
    // 2. Expand ranges like a-z
    expanded = expandRanges(expanded);
    rule.expandedRegex = expanded;

    // 3. Tokenize into internal tokens
    vector<RegexToken> rawTokens = tokenizeRegex(expanded);

    // 4. Insert explicit concatenation operators
    rule.infix = insertConcatenation(rawTokens);

    // 5. Convert infix to postfix
    rule.postfix = infixToPostfix(rule.infix);

    return rule;
}
