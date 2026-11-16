#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iosfwd>

using namespace std;

struct DFAState {
    int id;                             //! Unique identifier for the state
    bool isAccepting = false;           //! Indicates if this state is an accepting state
    string tokenName = "";              //! Name of the token class if this is an accepting state
    int priority = -1;                  //! Priority of the token class (lower number = higher priority)
    map<char, int> transitions;         //! Maps input characters to the next state ID (for transitions)
    set<int> nfaStates;                 //! Set of NFA states that this DFA state represents (used during subset construction)

    explicit DFAState(int stateId = 0) : id(stateId) {}   //? Constructor to initialize the state with a unique ID
};


class DFA {
public:
    static const int NO_STATE = -1;

    int startStateId = 0;
    vector<DFAState> states;
    set<char> alphabet;             //! Set of all input characters that the DFA recognizes (the alphabet)

    DFA() = default;

    //? Adds a new state with a unique ID and returns its ID
    int addState();

    //? Adds a character transition: from -> input -> to
    void addTransition(int from, char input, int to);

    //? Marks a state as accepting with its associated token class and priority
    void setAccepting(int stateId, const string& tokenName, int priority);

    //? Returns the next state, or NO_STATE if there is no transition (or invalid state)
    int getNextState(int stateId, char input) const;

    //? Returns true if the state is accepting
    bool isAccepting(int stateId) const;

    //? Returns the token name of an accepting state ("" if not accepting)
    string getAcceptingToken(int stateId) const;

    //? Returns total number of states
    size_t numStates() const { return states.size(); }

    //? Returns a list of all accepting state IDs
    vector<int> getAcceptingStates() const;

    //? Checks if a state ID is within valid bounds
    bool isValidState(int stateId) const {
        return stateId >= 0 && stateId < static_cast<int>(states.size());
    }

    //? Matches the input string against the DFA and returns the token name of the matched accepting state (or "" if no match)  (for testing purposes)
    string matchWhole(const string& input) const;

    //? Prints summary information about the DFA
    void printSummary() const;

    //? Prints the transition table as an aligned matrix (rows = states, columns = alphabet)
    void printTransitionTable(ostream& out) const;

    //? Writes the transition table to a CSV file.
    bool saveTransitionTableCSV(const string& filePath) const;

    //? Writes the transition table (aligned text) to a file.
    bool saveTransitionTableText(const string& filePath) const;
};
