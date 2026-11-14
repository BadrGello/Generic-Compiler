#include "../include/NFA.h"
#include <iostream>
#include <iomanip>

using namespace std;

int NFA::addState() {
    int newId = static_cast<int>(states.size());
    states.emplace_back(newId);
    return newId;
}

void NFA::addTransition(int from, char input, int to) {
    if (!isValidState(from) || !isValidState(to)) return;
    states[from].transitions[input].insert(to);
    alphabet.insert(input);
}

void NFA::addEpsilonTransition(int from, int to) {
    if (!isValidState(from) || !isValidState(to)) return;
    states[from].epsilonTransitions.insert(to);
}

void NFA::setAccepting(int stateId, const string& tokenName, int priority) {
    if (!isValidState(stateId)) return;
    states[stateId].isAccepting = true;
    states[stateId].tokenName = tokenName;
    states[stateId].priority = priority;
}

vector<int> NFA::getAcceptingStates() const {
    vector<int> accepting;
    for (const auto& s : states) {
        if (s.isAccepting) {
            accepting.push_back(s.id);
        }
    }
    return accepting;
}

void NFA::printSummary() const {
    cout << "NFA Summary:" << endl;
    cout << "  Total States: " << states.size() << endl;
    cout << "  Start State:  " << startStateId << endl;
    cout << "  Alphabet Size: " << alphabet.size() << " characters" << endl;

    vector<int> acc = getAcceptingStates();
    cout << "  Accepting States (" << acc.size() << "):" << endl;
    for (int id : acc) {
        const auto& s = states[id];
        cout << "    State " << setw(4) << s.id 
             << " -> Token: " << setw(10) << left << s.tokenName 
             << " (Priority " << s.priority << ")" << endl;
    }
}
