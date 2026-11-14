#include "../include/RuleParser.h"
#include "../include/NFABuilder.h"
#include "../include/DFABuilder.h"
#include "../include/DFAMinimizer.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <algorithm>

using namespace std;

void testMinimizeSmall() {
    cout << "\n--- TEST 1: MINIMIZATION ON A HAND-BUILT DFA ---" << endl;

    // Classic example: states 1 and 2 are equivalent (both accept "T" and behave the same).
    // 0 -a-> 1, 0 -b-> 2, 1 -a-> 3, 2 -a-> 3, 1 -b-> 1, 2 -b-> 2, 3 accepting "T"
    // Accepting: 1? no. Make 3 accepting; 1 and 2 are non-accepting and equivalent.
    DFA d;
    for (int i = 0; i < 4; i++) d.addState();
    d.addTransition(0, 'a', 1);
    d.addTransition(0, 'b', 2);
    d.addTransition(1, 'a', 3);
    d.addTransition(2, 'a', 3);
    d.addTransition(1, 'b', 1);
    d.addTransition(2, 'b', 2);
    d.setAccepting(3, "T", 0);

    DFA m = DFAMinimizer::minimize(d);
    assert(m.numStates() == 3);
    cout << " [PASS] equivalent states 1 and 2 merged (4 -> 3 states)." << endl;
    assert(m.matchWhole("aa") == "T");
    assert(m.matchWhole("bba") == "T");
    assert(m.matchWhole("ba") == "T");
    assert(m.matchWhole("a").empty());
    assert(m.matchWhole("ab").empty());
    cout << " [PASS] minimized DFA accepts the same language." << endl;

    // Different token classes must NOT be merged even if their transitions look alike
    DFA e;
    for (int i = 0; i < 3; i++) e.addState();
    e.addTransition(0, 'x', 1);
    e.addTransition(0, 'y', 2);
    e.setAccepting(1, "A", 0);
    e.setAccepting(2, "B", 1);
    DFA me = DFAMinimizer::minimize(e);
    assert(me.numStates() == 3);
    assert(me.matchWhole("x") == "A");
    assert(me.matchWhole("y") == "B");
    cout << " [PASS] accepting states of different token classes stay separate." << endl;

    // Unreachable states are dropped
    DFA u;
    for (int i = 0; i < 3; i++) u.addState();
    u.addTransition(0, 'a', 1);
    u.addTransition(2, 'a', 1); // state 2 unreachable
    u.setAccepting(1, "T", 0);
    DFA mu = DFAMinimizer::minimize(u);
    assert(mu.numStates() == 2);
    cout << " [PASS] unreachable states are removed." << endl;
}

void testMinimizeFullRules(const string& rulesPath) {
    cout << "\n--- TEST 2: MINIMIZATION ON THE ASSIGNMENT RULES ---" << endl;

    RuleParser parser;
    vector<TokenRule> rules = parser.parseFile(rulesPath);
    NFA nfa = NFABuilder::buildCombinedNFA(rules);
    DFA dfa = DFABuilder::buildFromNFA(nfa);
    DFA min = DFAMinimizer::minimize(dfa);

    cout << "Combined NFA states : " << nfa.numStates() << endl;
    cout << "DFA states          : " << dfa.numStates() << endl;
    cout << "Minimal DFA states  : " << min.numStates() << endl;
    min.printSummary();

    assert(min.numStates() <= dfa.numStates());
    assert(min.startStateId == 0);
    assert(min.alphabet == dfa.alphabet);
    cout << " [PASS] minimal DFA is not larger than the DFA; start state is 0." << endl;

    // Every token class from the rules still has at least one accepting state
    set<string> expected, found;
    for (const auto& r : rules) expected.insert(r.name);
    for (const auto& s : min.states) if (s.isAccepting) found.insert(s.tokenName);
    assert(expected == found);
    cout << " [PASS] all " << expected.size() << " token classes are still present." << endl;

    // Minimality check: no two states are equivalent any more (re-minimizing changes nothing)
    DFA again = DFAMinimizer::minimize(min);
    assert(again.numStates() == min.numStates());
    cout << " [PASS] minimizing again does not shrink it (already minimal)." << endl;

    // Same behavior as the original DFA on random strings
    const string chars = "abiflotwenhsdcr019.E<>=!+-*/;,(){} ";
    unsigned seed = 987654;
    auto next = [&]() { seed = seed * 1103515245u + 12345u; return (seed >> 16) & 0x7fff; };
    int checked = 0;
    for (int i = 0; i < 30000; i++) {
        int len = next() % 9;
        string s;
        for (int j = 0; j < len; j++) s += chars[next() % chars.size()];
        string a = dfa.matchWhole(s);
        string b = min.matchWhole(s);
        if (a != b) {
            cout << " [FAIL] mismatch on \"" << s << "\": DFA='" << a << "' MIN='" << b << "'" << endl;
        }
        assert(a == b);
        checked++;
    }
    cout << " [PASS] minimal DFA behaves exactly like the DFA on " << checked << " random strings." << endl;

    // Spot checks
    assert(min.matchWhole("while") == "while");
    assert(min.matchWhole("whilex") == "id");
    assert(min.matchWhole("12.5E3") == "num");
    assert(min.matchWhole("!=") == "relop");
    assert(min.matchWhole("pass") == "id");
    cout << " [PASS] spot checks (while / whilex / 12.5E3 / != / pass)." << endl;

    // Export the table: both a text file and a CSV file
    assert(min.saveTransitionTableText("minimal_dfa_table.txt"));
    assert(min.saveTransitionTableCSV("minimal_dfa_table.csv"));
    cout << " [PASS] transition table written to minimal_dfa_table.txt and minimal_dfa_table.csv." << endl;

    cout << "\n--- MINIMAL DFA TRANSITION TABLE ---" << endl;
    min.printTransitionTable(cout);
}

int main() {
    cout << "====================================================" << endl;
    cout << "        LEXICAL ANALYZER GENERATOR - STEP 3 TESTS   " << endl;
    cout << "====================================================" << endl;

    string rulesPath = "tests/lexical_rules.txt";
    {
        vector<string> candidates = {
            "tests/lexical_rules.txt",
            "../tests/lexical_rules.txt",
            "lexical_rules.txt",
            "../../tests/lexical_rules.txt"
        };
        for (const auto& p : candidates) {
            ifstream f(p);
            if (f.good()) { rulesPath = p; break; }
        }
    }

    testMinimizeSmall();
    testMinimizeFullRules(rulesPath);

    cout << "\nALL STEP 3 TESTS PASSED SUCCESSFULLY!" << endl;
    cout << "====================================================" << endl;
    return 0;
}
