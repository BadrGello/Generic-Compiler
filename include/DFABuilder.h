#pragma once

#include "NFA.h"
#include "DFA.h"
#include <set>

using namespace std;

class DFABuilder {
public:
    // Converts an NFA (e.g. the Combined Master NFA from Step 1) to a DFA using
    // Subset Construction. Accepting DFA states get the token with the best
    // (lowest) priority among all accepting NFA states they contain.
    static DFA buildFromNFA(const NFA& nfa);

    // Epsilon-closure of a set of NFA states
    static set<int> epsilonClosure(const NFA& nfa, const set<int>& states);

    // move(T, c): set of NFA states reachable from T on character c (no closure)
    static set<int> move(const NFA& nfa, const set<int>& states, char c);
};
