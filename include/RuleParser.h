#pragma once

#include "RegexToken.h"
#include <string>
#include <vector>
#include <map>

using namespace std;

// Represents a parsed token rule from the rules file
struct TokenRule {
    string name;                // Token name: "id", "num", "int", ";", etc.
    int priority;               // Lower number = higher priority
    string rawRegex;            // Original regex string from the file
    string expandedRegex;       // Regex after definition and range expansion
    vector<RegexToken> infix;   // Infix tokens with explicit concatenation inserted
    vector<RegexToken> postfix; // Postfix tokens ready for Thompson's construction

    TokenRule(const string& ruleName = "", int prio = 0, const string& raw = "")
        : name(ruleName), priority(prio), rawRegex(raw) {}
};

class RuleParser {
public:
    RuleParser() = default;

    // Parses the lexical rules file and returns the list of TokenRules in priority order
    vector<TokenRule> parseFile(const string& filePath);

    // Parses from a string directly (useful for testing)
    vector<TokenRule> parseContent(const string& content);

    // Access to stored regular definitions (name -> expanded definition)
    const map<string, string>& getDefinitions() const {
        return definitions;
    }

    // Public helper methods (can be tested individually)
    static string expandRanges(const string& text);
    static vector<RegexToken> tokenizeRegex(const string& regexStr);
    static vector<RegexToken> insertConcatenation(const vector<RegexToken>& tokens);
    static vector<RegexToken> infixToPostfix(const vector<RegexToken>& infixTokens);

private:
    map<string, string> definitions;               // Stored definitions (LHS -> RHS)
    vector<pair<string, string>> definitionOrder;   // Preserves the order definitions appeared in

    string trim(const string& str);
    string expandDefinitions(const string& str);
    void processKeywordLine(const string& line, vector<TokenRule>& rules, int& nextPriority);
    void processPunctuationLine(const string& line, vector<TokenRule>& rules, int& nextPriority);
    void processDefinitionLine(const string& line);
    void processExpressionLine(const string& line, vector<TokenRule>& rules, int& nextPriority);
    TokenRule buildTokenRule(const string& name, int priority, const string& rawExpr);
};
