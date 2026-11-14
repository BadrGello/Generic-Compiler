#pragma once

#include "NFA.h"
#include "RuleParser.h"
#include <vector>

using namespace std;

// An NFA Fragment represents a sub-automaton with a single entry and single exit state
struct NFAFragment {
    int start;
    int accept;

    NFAFragment(int s = 0, int a = 0) : start(s), accept(a) {}
};

class NFABuilder {
public:
    // Builds an NFA for a single TokenRule using Thompson's construction on its postfix tokens
    static NFA buildFromRule(const TokenRule& rule);

    // Builds a single Combined Master NFA combining all TokenRules with a unified start state
    static NFA buildCombinedNFA(const vector<TokenRule>& rules);

    // Thompson's Construction Primitives
    static NFAFragment createLiteral(NFA& nfa, char c);
    static NFAFragment createLambda(NFA& nfa);
    static NFAFragment createConcat(NFA& nfa, const NFAFragment& first, const NFAFragment& second);
    static NFAFragment createUnion(NFA& nfa, const NFAFragment& first, const NFAFragment& second);
    static NFAFragment createStar(NFA& nfa, const NFAFragment& base);
    static NFAFragment createPlus(NFA& nfa, const NFAFragment& base);

private:
    // Helper to evaluate a postfix expression into an NFA
    static NFAFragment evaluatePostfix(NFA& nfa, const vector<RegexToken>& postfix);
};
