#include "../include/DFA.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

int DFA::addState() {
    int newId = static_cast<int>(states.size());
    states.emplace_back(newId);
    return newId;
}

void DFA::addTransition(int from, char input, int to) {
    if (!isValidState(from) || !isValidState(to)) return;
    states[from].transitions[input] = to;
    alphabet.insert(input);
}

void DFA::setAccepting(int stateId, const string& tokenName, int priority) {
    if (!isValidState(stateId)) return;
    states[stateId].isAccepting = true;
    states[stateId].tokenName = tokenName;
    states[stateId].priority = priority;
}

int DFA::getNextState(int stateId, char input) const {
    if (!isValidState(stateId)) return NO_STATE;
    auto it = states[stateId].transitions.find(input);
    if (it == states[stateId].transitions.end()) return NO_STATE;
    return it->second;
}

bool DFA::isAccepting(int stateId) const {
    return isValidState(stateId) && states[stateId].isAccepting;
}

string DFA::getAcceptingToken(int stateId) const {
    if (!isAccepting(stateId)) return "";
    return states[stateId].tokenName;
}

vector<int> DFA::getAcceptingStates() const {
    vector<int> accepting;
    for (const auto& s : states) {
        if (s.isAccepting) accepting.push_back(s.id);
    }
    return accepting;
}

string DFA::matchWhole(const string& input) const {
    if (states.empty()) return "";
    int cur = startStateId;
    for (char c : input) {
        cur = getNextState(cur, c);
        if (cur == NO_STATE) return "";
    }
    return getAcceptingToken(cur);
}

void DFA::printSummary() const {
    cout << "DFA Summary:" << endl;
    cout << "  Total States: " << states.size() << endl;
    cout << "  Start State:  " << startStateId << endl;
    cout << "  Alphabet Size: " << alphabet.size() << " characters" << endl;

    vector<int> acc = getAcceptingStates();
    cout << "  Accepting States (" << acc.size() << "):" << endl;
    for (int id : acc) {
        const auto& s = states[id];
        cout << "    State " << right << setw(4) << s.id
             << " -> Token: " << setw(10) << left << s.tokenName
             << " (Priority " << s.priority << ")" << endl;
    }
}

// Column header for a character (non-printable / special ones are made readable)
static string charLabel(char c) {
    if (c == ',') return "','";   // keep commas distinguishable in CSV
    if (c == ' ') return "' '";
    return string(1, c);
}

void DFA::printTransitionTable(ostream& out) const {
    const int stateW = 8;
    const int colW = 5;
    const int tokenW = 10;

    out << left << setw(stateW) << "State";
    for (char c : alphabet) out << setw(colW) << charLabel(c);
    out << setw(tokenW) << "Token" << endl;
    out << string(stateW + colW * alphabet.size() + tokenW, '-') << endl;

    for (const auto& s : states) {
        string label = (s.id == startStateId ? ">" : " ") + to_string(s.id);
        if (s.isAccepting) label += "*";
        out << left << setw(stateW) << label;
        for (char c : alphabet) {
            int next = getNextState(s.id, c);
            out << setw(colW) << (next == NO_STATE ? string("-") : to_string(next));
        }
        out << setw(tokenW) << (s.isAccepting ? s.tokenName : string("")) << endl;
    }
    out << "(> = start state, * = accepting state, - = no transition)" << endl;
}

bool DFA::saveTransitionTableText(const string& filePath) const {
    ofstream file(filePath);
    if (!file.is_open()) return false;
    printTransitionTable(file);
    return true;
}

bool DFA::saveTransitionTableCSV(const string& filePath) const {
    ofstream file(filePath);
    if (!file.is_open()) return false;

    // Header. Commas/quotes in the header are wrapped in quotes to stay valid CSV.
    file << "State,Start,Accepting,Token";
    for (char c : alphabet) {
        string label(1, c);
        if (c == ',') label = "\",\"";
        else if (c == '"') label = "\"\"\"\"";
        file << "," << label;
    }
    file << "\n";

    for (const auto& s : states) {
        string token = s.isAccepting ? s.tokenName : "";
        if (token == ",") token = "\",\"";
        file << s.id << "," << (s.id == startStateId ? 1 : 0) << ","
             << (s.isAccepting ? 1 : 0) << "," << token;
        for (char c : alphabet) {
            int next = getNextState(s.id, c);
            file << "," << (next == NO_STATE ? string("") : to_string(next));
        }
        file << "\n";
    }
    return true;
}
