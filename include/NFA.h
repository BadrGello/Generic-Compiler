#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

// Represents a single state in an NFA
struct NFAState {
    int id;
    bool isAccepting = false;
    string tokenName = "";   // e.g. "id", "num", "int", ";"
    int priority = -1;       // Lower number = higher priority

    // Character transitions: character -> set of target state IDs
    map<char, set<int>> transitions;

    // Epsilon / Lambda transitions: set of target state IDs reached for free
    set<int> epsilonTransitions;

    explicit NFAState(int stateId = 0) : id(stateId) {}
};

// Represents a Non-deterministic Finite Automaton
class NFA {
public:
    int startStateId = 0;
    vector<NFAState> states;
    set<char> alphabet; // All unique input characters recognized by this NFA

    NFA() = default;

    // Adds a new state with a unique ID and returns its ID
    int addState();

    // Adds a character transition: from -> input -> to
    void addTransition(int from, char input, int to);

    // Adds an epsilon (\L) transition: from -> to
    void addEpsilonTransition(int from, int to);

    // Marks a state as accepting with its associated token class and priority
    void setAccepting(int stateId, const string& tokenName, int priority);

    // Returns total number of states in the NFA
    size_t numStates() const { return states.size(); }

    // Returns a list of all accepting state IDs
    vector<int> getAcceptingStates() const;

    // Checks if a state ID is within valid bounds
    bool isValidState(int stateId) const {
        return stateId >= 0 && stateId < static_cast<int>(states.size());
    }

    // Prints summary information about the NFA
    void printSummary() const;
};
