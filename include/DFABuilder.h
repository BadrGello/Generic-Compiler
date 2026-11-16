#pragma once

#include "NFA.h"
#include "DFA.h"
#include <set>

using namespace std;

class DFABuilder {
public:
    //! Builds a DFA from an NFA using the subset construction algorithm.
    static DFA buildFromNFA(const NFA& nfa);

    //? Epsilon-closure of a set of NFA states
    static set<int> epsilonClosure(const NFA& nfa, const set<int>& states);

    //? move(T, c): set of NFA states reachable from T on character c (no closure)
    static set<int> move(const NFA& nfa, const set<int>& states, char c);
};
