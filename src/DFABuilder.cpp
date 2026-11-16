#include "../include/DFABuilder.h"
#include <map>
#include <queue>
#include <stack>

using namespace std;

set<int> DFABuilder::epsilonClosure(const NFA& nfa, const set<int>& states) {
    set<int> closure = states;
    stack<int> work;                            //? Has states whose epsilon transitions have not been explored yet
    for (int s : states) work.push(s);

    while (!work.empty()) {
        int cur = work.top();
        work.pop();
        if (!nfa.isValidState(cur)) continue;
        for (int next : nfa.states[cur].epsilonTransitions) {
            if (closure.insert(next).second) {       //? If next was newly added to closure, explore its epsilon transitions
                work.push(next);
            }
        }
    }
    return closure;
}

set<int> DFABuilder::move(const NFA& nfa, const set<int>& states, char c) {
    set<int> result;
    for (int s : states) {
        if (!nfa.isValidState(s)) continue;
        auto it = nfa.states[s].transitions.find(c);
        if (it == nfa.states[s].transitions.end()) continue;
        result.insert(it->second.begin(), it->second.end());
    }
    return result;
}

DFA DFABuilder::buildFromNFA(const NFA& nfa) {        //? Subset construction algorithm to convert an NFA to a DFA
    DFA dfa;
    if (nfa.states.empty()) return dfa;

    map<set<int>, int> subsetToId;                  //! NFA-state-set -> DFA state ID
    queue<int> unmarked;                            //! DFA states whose transitions are not computed yet

    //? Creates the DFA state for a subset (if new) and resolves its accepting info.
    auto getOrCreate = [&](const set<int>& subset) -> int {
        auto it = subsetToId.find(subset);
        if (it != subsetToId.end()) return it->second;       //? Already exists, return its ID

        int id = dfa.addState();
        subsetToId[subset] = id;
        dfa.states[id].nfaStates = subset;

        int bestPriority = -1;
        string bestName;
        bool found = false;
        for (int s : subset) {
            const NFAState& ns = nfa.states[s];
            if (!ns.isAccepting) continue;
            if (!found || ns.priority < bestPriority) {       //? Found a better (lower) priority accepting state
                found = true;
                bestPriority = ns.priority;
                bestName = ns.tokenName;
            }
        }
        if (found) dfa.setAccepting(id, bestName, bestPriority);

        unmarked.push(id);
        return id;
    };

    set<int> startSet = epsilonClosure(nfa, set<int>{nfa.startStateId});      //? Epsilon-closure of the NFA's start state
    dfa.startStateId = getOrCreate(startSet);

    while (!unmarked.empty()) {         //? While there are unmarked DFA states, compute their transitions
        int cur = unmarked.front();
        unmarked.pop();
        set<int> curSet = dfa.states[cur].nfaStates; // copy: states vector may grow

        for (char c : nfa.alphabet) {
            set<int> moved = move(nfa, curSet, c);
            if (moved.empty()) continue; // no transition -> implicit dead state
            set<int> target = epsilonClosure(nfa, moved);
            int targetId = getOrCreate(target);
            dfa.addTransition(cur, c, targetId);
        }
    }

    //? Keep the alphabet identical to the NFA's, even if some char is never used
    dfa.alphabet.insert(nfa.alphabet.begin(), nfa.alphabet.end());
    return dfa;
}
