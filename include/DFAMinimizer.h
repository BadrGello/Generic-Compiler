#pragma once

#include "DFA.h"

using namespace std;

class DFAMinimizer {
public:
    // Minimizes a DFA using partition refinement (Moore's algorithm).
    // The initial partition separates:
    //   - non-accepting states, and
    //   - accepting states grouped BY TOKEN CLASS (so different tokens are never merged).
    // Missing transitions are treated as going to an implicit dead state.
    // The result is renumbered in BFS order from the start state (start state = 0).
    static DFA minimize(const DFA& dfa);
};
