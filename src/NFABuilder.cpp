#include "../include/NFABuilder.h"
#include <stack>
#include <iostream>

using namespace std;

// 1. Literal Character: creates start -> 'c' -> accept
NFAFragment NFABuilder::createLiteral(NFA& nfa, char c) {
    int start = nfa.addState();
    int accept = nfa.addState();
    nfa.addTransition(start, c, accept);
    return NFAFragment(start, accept);
}

// 2. Lambda (\L / epsilon): creates start -> epsilon -> accept
NFAFragment NFABuilder::createLambda(NFA& nfa) {
    int start = nfa.addState();
    int accept = nfa.addState();
    nfa.addEpsilonTransition(start, accept);
    return NFAFragment(start, accept);
}

// 3. Concatenation (A . B): connects A's accept state to B's start state via epsilon
NFAFragment NFABuilder::createConcat(NFA& nfa, const NFAFragment& first, const NFAFragment& second) {
    nfa.addEpsilonTransition(first.accept, second.start);
    return NFAFragment(first.start, second.accept);
}

// 4. Union (A | B): new start branches to both; both join to new accept
NFAFragment NFABuilder::createUnion(NFA& nfa, const NFAFragment& first, const NFAFragment& second) {
    int newStart = nfa.addState();
    int newAccept = nfa.addState();

    nfa.addEpsilonTransition(newStart, first.start);
    nfa.addEpsilonTransition(newStart, second.start);
    nfa.addEpsilonTransition(first.accept, newAccept);
    nfa.addEpsilonTransition(second.accept, newAccept);

    return NFAFragment(newStart, newAccept);
}

// 5. Kleene Star (A*): loopback (repeat) and bypass (zero occurrences)
NFAFragment NFABuilder::createStar(NFA& nfa, const NFAFragment& base) {
    int newStart = nfa.addState();
    int newAccept = nfa.addState();

    nfa.addEpsilonTransition(newStart, base.start);
    nfa.addEpsilonTransition(base.accept, newAccept);
    nfa.addEpsilonTransition(base.accept, base.start); // Repeat loopback
    nfa.addEpsilonTransition(newStart, newAccept);     // 0 times bypass

    return NFAFragment(newStart, newAccept);
}

// 6. Positive Closure (A+): loopback (repeat) with NO bypass (at least 1 match)
NFAFragment NFABuilder::createPlus(NFA& nfa, const NFAFragment& base) {
    int newStart = nfa.addState();
    int newAccept = nfa.addState();

    nfa.addEpsilonTransition(newStart, base.start);
    nfa.addEpsilonTransition(base.accept, newAccept);
    nfa.addEpsilonTransition(base.accept, base.start); // Repeat loopback

    return NFAFragment(newStart, newAccept);
}

// Evaluates postfix tokens using an NFAFragment stack (standard Thompson evaluation)
NFAFragment NFABuilder::evaluatePostfix(NFA& nfa, const vector<RegexToken>& postfix) {
    stack<NFAFragment> st;

    for (const auto& token : postfix) {
        switch (token.type) {
            case TokenType::LITERAL:
                st.push(createLiteral(nfa, token.value));
                break;

            case TokenType::LAMBDA:
                st.push(createLambda(nfa));
                break;

            case TokenType::CONCAT: {
                if (st.size() < 2) {
                    cerr << "Error: Not enough operands for CONCAT in NFA construction." << endl;
                    break;
                }
                NFAFragment second = st.top(); st.pop();
                NFAFragment first = st.top(); st.pop();
                st.push(createConcat(nfa, first, second));
                break;
            }

            case TokenType::UNION: {
                if (st.size() < 2) {
                    cerr << "Error: Not enough operands for UNION in NFA construction." << endl;
                    break;
                }
                NFAFragment second = st.top(); st.pop();
                NFAFragment first = st.top(); st.pop();
                st.push(createUnion(nfa, first, second));
                break;
            }

            case TokenType::STAR: {
                if (st.empty()) {
                    cerr << "Error: Not enough operands for STAR in NFA construction." << endl;
                    break;
                }
                NFAFragment base = st.top(); st.pop();
                st.push(createStar(nfa, base));
                break;
            }

            case TokenType::PLUS: {
                if (st.empty()) {
                    cerr << "Error: Not enough operands for PLUS in NFA construction." << endl;
                    break;
                }
                NFAFragment base = st.top(); st.pop();
                st.push(createPlus(nfa, base));
                break;
            }

            default:
                break;
        }
    }

    if (st.empty()) {
        cerr << "Error: Postfix evaluation produced no NFA fragment." << endl;
        int emptyState = nfa.addState();
        return NFAFragment(emptyState, emptyState);
    }

    return st.top();
}

// Builds an NFA for a single token rule
NFA NFABuilder::buildFromRule(const TokenRule& rule) {
    NFA nfa;
    if (rule.postfix.empty()) return nfa;

    NFAFragment frag = evaluatePostfix(nfa, rule.postfix);
    nfa.startStateId = frag.start;
    nfa.setAccepting(frag.accept, rule.name, rule.priority);
    return nfa;
}

// Combines all token rule NFAs into one Master NFA with a single global start state
NFA NFABuilder::buildCombinedNFA(const vector<TokenRule>& rules) {
    NFA combined;
    int masterStart = combined.addState();
    combined.startStateId = masterStart;

    for (const auto& rule : rules) {
        if (rule.postfix.empty()) continue;

        NFAFragment frag = evaluatePostfix(combined, rule.postfix);
        combined.setAccepting(frag.accept, rule.name, rule.priority);
        combined.addEpsilonTransition(masterStart, frag.start);
    }

    return combined;
}
