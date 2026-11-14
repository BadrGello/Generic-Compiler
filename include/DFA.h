#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iosfwd>

using namespace std;

// Represents a single state in a DFA
struct DFAState {
    int id;
    bool isAccepting = false;
    string tokenName = "";   // Winning token class (e.g. "id", "num", "int", ";")
    int priority = -1;       // Priority of the winning token (lower = higher priority)

    // Deterministic transitions: character -> exactly one target state ID.
    // A missing entry means "no transition" (implicit dead state).
    map<char, int> transitions;

    // The set of NFA state IDs this DFA state represents (filled by subset construction).
    // Empty for states of a minimized DFA.
    set<int> nfaStates;

    explicit DFAState(int stateId = 0) : id(stateId) {}
};

// Represents a Deterministic Finite Automaton
class DFA {
public:
    static const int NO_STATE = -1; // Returned when there is no transition

    int startStateId = 0;
    vector<DFAState> states;
    set<char> alphabet; // All unique input characters used by this DFA

    DFA() = default;

    // Adds a new state with a unique ID and returns its ID
    int addState();

    // Adds a character transition: from -> input -> to
    void addTransition(int from, char input, int to);

    // Marks a state as accepting with its associated token class and priority
    void setAccepting(int stateId, const string& tokenName, int priority);

    // Returns the next state, or NO_STATE if there is no transition (or invalid state)
    int getNextState(int stateId, char input) const;

    // Returns true if the state is accepting
    bool isAccepting(int stateId) const;

    // Returns the token name of an accepting state ("" if not accepting)
    string getAcceptingToken(int stateId) const;

    // Returns total number of states
    size_t numStates() const { return states.size(); }

    // Returns a list of all accepting state IDs
    vector<int> getAcceptingStates() const;

    // Checks if a state ID is within valid bounds
    bool isValidState(int stateId) const {
        return stateId >= 0 && stateId < static_cast<int>(states.size());
    }

    // Runs the DFA on a whole string. Returns the token name if the ENTIRE string
    // ends in an accepting state, otherwise "" (handy for testing; the real
    // longest-prefix scanner is built on top of getNextState in Step 4).
    string matchWhole(const string& input) const;

    // Prints summary information about the DFA
    void printSummary() const;

    // Prints the transition table as an aligned matrix (rows = states, columns = alphabet)
    void printTransitionTable(ostream& out) const;

    // Writes the transition table to a CSV file. Returns false if the file can't be opened.
    bool saveTransitionTableCSV(const string& filePath) const;

    // Writes the transition table (aligned text) to a file. Returns false on failure.
    bool saveTransitionTableText(const string& filePath) const;
};
