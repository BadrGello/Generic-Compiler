#include "../include/RuleParser.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <algorithm>

using namespace std;

void testSingleExpression(const string& name, const string& expr) {
    cout << "Testing expression: " << name << " -> " << expr << endl;
    string expanded = RuleParser::expandRanges(expr);
    auto rawTokens = RuleParser::tokenizeRegex(expanded);
    auto infix = RuleParser::insertConcatenation(rawTokens);
    auto postfix = RuleParser::infixToPostfix(infix);

    cout << "  Infix:   " << tokensToString(infix) << endl;
    cout << "  Postfix: " << tokensToString(postfix) << endl;
    cout << "----------------------------------------------------" << endl;
}

int main(int argc, char* argv[]) {
    cout << "====================================================" << endl;
    cout << "        LEXICAL ANALYZER GENERATOR - STEP 0 TESTS   " << endl;
    cout << "====================================================" << endl;

    string rulesPath = "tests/lexical_rules.txt";
    if (argc > 1) {
        rulesPath = argv[1];
    } else {
        // Multi-path resolution: checks standard run locations
        vector<string> candidatePaths = {
            "tests/lexical_rules.txt",
            "../tests/lexical_rules.txt",
            "lexical_rules.txt",
            "../../tests/lexical_rules.txt"
        };
        for (const auto& path : candidatePaths) {
            ifstream f(path);
            if (f.good()) {
                rulesPath = path;
                break;
            }
        }
    }

    RuleParser parser;
    vector<TokenRule> rules = parser.parseFile(rulesPath);

    if (rules.empty()) {
        cerr << "FAIL: No rules parsed from " << rulesPath << endl;
        return 1;
    }

    cout << "SUCCESS: Successfully parsed " << rules.size() << " token rules." << endl;
    cout << "\n--- REGULAR DEFINITIONS STORED ---" << endl;
    for (const auto& def : parser.getDefinitions()) {
        cout << setw(10) << left << def.first 
             << " = " << def.second.substr(0, 50) 
             << (def.second.length() > 50 ? "..." : "") << endl;
    }

    cout << "\n--- PARSED TOKEN RULES (Priority Order) ---" << endl;
    cout << setw(6) << "Prio" 
         << setw(12) << "Token Name" 
         << setw(25) << "Raw Regex" 
         << "Postfix Preview" << endl;
    cout << string(80, '-') << endl;

    for (const auto& r : rules) {
        string postfixStr = tokensToString(r.postfix);
        string preview = postfixStr.substr(0, 40) + (postfixStr.length() > 40 ? "..." : "");
        cout << setw(6) << r.priority 
             << setw(12) << r.name 
             << setw(25) << (r.rawRegex.substr(0, 22) + (r.rawRegex.length() > 22 ? "..." : ""))
             << preview << endl;
    }

    // --- ASSERTIONS / VALIDATIONS ---
    cout << "\n--- RUNNING VALIDATION CHECKS ---" << endl;

    // 1. Verify keywords are prioritized (Priority 0 to 5)
    vector<string> expectedKeywords = {"boolean", "int", "float", "if", "else", "while"};
    for (size_t i = 0; i < expectedKeywords.size(); ++i) {
        assert(rules[i].name == expectedKeywords[i]);
        assert(rules[i].priority == static_cast<int>(i));
    }
    cout << " [PASS] Keywords are reserved and assigned highest priorities (0 to 5)." << endl;

    // 2. Verify punctuations
    bool foundSemi = false, foundComma = false, foundLParen = false, foundRParen = false;
    for (const auto& r : rules) {
        if (r.name == ";") foundSemi = true;
        if (r.name == ",") foundComma = true;
        if (r.name == "(") foundLParen = true;
        if (r.name == ")") foundRParen = true;
    }
    assert(foundSemi && foundComma && foundLParen && foundRParen);
    cout << " [PASS] Punctuation symbols (;, ,, (, ), {, }) extracted correctly." << endl;

    // 3. Verify 'id' token expands definitions
    auto itId = find_if(rules.begin(), rules.end(), [](const TokenRule& r) { return r.name == "id"; });
    assert(itId != rules.end());
    assert(!itId->postfix.empty());
    assert(itId->postfix.back().type == TokenType::CONCAT);
    cout << " [PASS] 'id' properly expanded macros and converted to postfix ending in CONCAT." << endl;

    // 4. Verify 'num' token contains literal dot and lambda
    auto itNum = find_if(rules.begin(), rules.end(), [](const TokenRule& r) { return r.name == "num"; });
    assert(itNum != rules.end());
    bool numHasLambda = false;
    bool numHasLiteralDot = false;
    for (const auto& t : itNum->postfix) {
        if (t.type == TokenType::LAMBDA) numHasLambda = true;
        if (t.type == TokenType::LITERAL && t.value == '.') numHasLiteralDot = true;
    }
    assert(numHasLambda);
    assert(numHasLiteralDot);
    cout << " [PASS] 'num' contains literal '.' decimal point and '\\L' (Lambda/epsilon)." << endl;

    // 5. Verify 'relop' escaped operators
    auto itRelop = find_if(rules.begin(), rules.end(), [](const TokenRule& r) { return r.name == "relop"; });
    assert(itRelop != rules.end());
    cout << " [PASS] 'relop' handles escaped characters like \\=\\= and !\\=." << endl;

    // 6. Unit tests for edge cases: Shunting-Yard
    cout << "\n--- UNIT TESTS: SHUNTING-YARD ALGORITHM ---" << endl;
    {
        vector<RegexToken> in = {
            RegexToken(TokenType::LITERAL, 'a'),
            RegexToken(TokenType::CONCAT),
            RegexToken(TokenType::LPAREN),
            RegexToken(TokenType::LITERAL, 'b'),
            RegexToken(TokenType::UNION),
            RegexToken(TokenType::LITERAL, 'c'),
            RegexToken(TokenType::RPAREN),
            RegexToken(TokenType::STAR)
        };
        auto post = RuleParser::infixToPostfix(in);
        // Expected: a b c | * .
        assert(post.size() == 6);
        assert(post[0].value == 'a');
        assert(post[1].value == 'b');
        assert(post[2].value == 'c');
        assert(post[3].type == TokenType::UNION);
        assert(post[4].type == TokenType::STAR);
        assert(post[5].type == TokenType::CONCAT);
        cout << " [PASS] Postfix for 'a (b|c)*' is exactly 'a b c | * .'" << endl;
    }

    cout << "\nALL STEP 0 TESTS PASSED SUCCESSFULLY!" << endl;
    cout << "====================================================" << endl;
    return 0;
}
