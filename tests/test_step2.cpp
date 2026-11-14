#include "../include/RuleParser.h"
#include "../include/NFABuilder.h"
#include "../include/DFABuilder.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <algorithm>

using namespace std;

// Independent reference: simulate the NFA directly on a whole string and return
// the best-priority accepting token ("" if none). Used to cross-check the DFA.
static string simulateNFA(const NFA& nfa, const string& input) {
    set<int> cur = DFABuilder::epsilonClosure(nfa, set<int>{nfa.startStateId});
    for (char c : input) {
        cur = DFABuilder::epsilonClosure(nfa, DFABuilder::move(nfa, cur, c));
        if (cur.empty()) return "";
    }
    int best = -1;
    string name;
    for (int s : cur) {
        if (nfa.states[s].isAccepting && (best == -1 || nfa.states[s].priority < best)) {
            best = nfa.states[s].priority;
            name = nfa.states[s].tokenName;
        }
    }
    return name;
}

void testClosureAndMove() {
    cout << "\n--- TEST 1: EPSILON-CLOSURE & MOVE ---" << endl;

    // Hand-built NFA:  0 -e-> 1, 1 -e-> 2, 0 -a-> 3, 2 -a-> 4
    NFA nfa;
    for (int i = 0; i < 5; i++) nfa.addState();
    nfa.addEpsilonTransition(0, 1);
    nfa.addEpsilonTransition(1, 2);
    nfa.addTransition(0, 'a', 3);
    nfa.addTransition(2, 'a', 4);

    set<int> closure = DFABuilder::epsilonClosure(nfa, set<int>{0});
    assert((closure == set<int>{0, 1, 2}));
    cout << " [PASS] epsilon-closure({0}) = {0,1,2}." << endl;

    set<int> moved = DFABuilder::move(nfa, closure, 'a');
    assert((moved == set<int>{3, 4}));
    cout << " [PASS] move({0,1,2}, 'a') = {3,4}." << endl;

    assert(DFABuilder::move(nfa, closure, 'b').empty());
    cout << " [PASS] move on a character with no transition is empty." << endl;

    // Loop in epsilon transitions must terminate
    NFA loop;
    loop.addState(); loop.addState();
    loop.addEpsilonTransition(0, 1);
    loop.addEpsilonTransition(1, 0);
    assert((DFABuilder::epsilonClosure(loop, set<int>{0}) == set<int>{0, 1}));
    cout << " [PASS] epsilon-closure terminates on epsilon cycles." << endl;
}

void testSubsetConstructionSmall() {
    cout << "\n--- TEST 2: SUBSET CONSTRUCTION ON SMALL RULES ---" << endl;

    // Priority resolution: both 'if' (priority 0) and 'id' (priority 1) accept "if"
    RuleParser parser;
    vector<TokenRule> rules = parser.parseContent(
        "letter = a-z\n"
        "{ if }\n"
        "id: letter+\n");
    assert(rules.size() == 2);

    NFA nfa = NFABuilder::buildCombinedNFA(rules);
    DFA dfa = DFABuilder::buildFromNFA(nfa);

    assert(dfa.matchWhole("if") == "if");
    cout << " [PASS] \"if\" -> keyword 'if' (higher priority wins over 'id')." << endl;
    assert(dfa.matchWhole("i") == "id");
    assert(dfa.matchWhole("iff") == "id");
    assert(dfa.matchWhole("abc") == "id");
    cout << " [PASS] \"i\", \"iff\", \"abc\" -> 'id'." << endl;
    assert(dfa.matchWhole("").empty());
    assert(dfa.matchWhole("1").empty());
    cout << " [PASS] empty string and non-matching input are rejected." << endl;

    // Every DFA state must be deterministic by construction (map<char,int>)
    for (const auto& s : dfa.states) {
        for (const auto& kv : s.transitions) assert(dfa.isValidState(kv.second));
    }
    cout << " [PASS] every transition points to a valid DFA state." << endl;

    // Exactly one DFA state for the 'if' acceptance, and its name/priority are correct
    int ifStates = 0;
    for (const auto& s : dfa.states) {
        if (s.isAccepting && s.tokenName == "if") {
            ifStates++;
            assert(s.priority == 0);
        }
    }
    assert(ifStates == 1);
    cout << " [PASS] accepting state for 'if' has priority 0." << endl;
}

void testSubsetConstructionFullRules(const string& rulesPath) {
    cout << "\n--- TEST 3: SUBSET CONSTRUCTION ON THE ASSIGNMENT RULES ---" << endl;

    RuleParser parser;
    vector<TokenRule> rules = parser.parseFile(rulesPath);
    NFA nfa = NFABuilder::buildCombinedNFA(rules);
    DFA dfa = DFABuilder::buildFromNFA(nfa);

    cout << "Combined NFA states: " << nfa.numStates() << endl;
    dfa.printSummary();

    assert(dfa.numStates() > 0);
    assert(dfa.startStateId == 0);
    assert(dfa.alphabet == nfa.alphabet);
    cout << " [PASS] DFA built; start state is 0; alphabet equals the NFA alphabet." << endl;

    struct Case { string input; string expected; };
    vector<Case> cases = {
        {"int", "int"}, {"float", "float"}, {"boolean", "boolean"},
        {"if", "if"}, {"else", "else"}, {"while", "while"},
        {"integer", "id"}, {"iff", "id"}, {"whiles", "id"}, {"sum", "id"},
        {"pass", "id"}, {"x1", "id"}, {"abc123", "id"},
        {"10", "num"}, {"0", "num"}, {"12.5", "num"}, {"3.14E10", "num"}, {"7.0E2", "num"},
        {"==", "relop"}, {"!=", "relop"}, {">", "relop"}, {">=", "relop"},
        {"<", "relop"}, {"<=", "relop"},
        {"=", "assign"}, {"+", "addop"}, {"-", "addop"}, {"*", "mulop"}, {"/", "mulop"},
        {";", ";"}, {",", ","}, {"(", "("}, {")", ")"}, {"{", "{"}, {"}", "}"},
        // Things that must NOT match as a single whole token
        {"12.", ""}, {"1E5", ""}, {"3.E2", ""}, {"1abc", ""}, {"!", ""}, {"=>", ""},
        {"", ""}, {"a b", ""}, {"@", ""}
    };
    for (const auto& c : cases) {
        string got = dfa.matchWhole(c.input);
        if (got != c.expected) {
            cout << " [FAIL] \"" << c.input << "\" expected '" << c.expected
                 << "' got '" << got << "'" << endl;
        }
        assert(got == c.expected);
    }
    cout << " [PASS] " << cases.size() << " token / non-token strings classified correctly." << endl;

    // Cross-check DFA against direct NFA simulation on lots of generated strings
    const string chars = "abiflotwenhsdcr019.E<>=!+-*/;,(){} ";
    unsigned seed = 12345;
    auto next = [&]() { seed = seed * 1103515245u + 12345u; return (seed >> 16) & 0x7fff; };
    int checked = 0;
    for (int i = 0; i < 20000; i++) {
        int len = next() % 8;
        string s;
        for (int j = 0; j < len; j++) s += chars[next() % chars.size()];
        string a = simulateNFA(nfa, s);
        string b = dfa.matchWhole(s);
        if (a != b) {
            cout << " [FAIL] mismatch on \"" << s << "\": NFA='" << a << "' DFA='" << b << "'" << endl;
        }
        assert(a == b);
        checked++;
    }
    cout << " [PASS] DFA agrees with direct NFA simulation on " << checked << " random strings." << endl;

    // Every accepting DFA state's token must be a real token from the rules
    set<string> names;
    for (const auto& r : rules) names.insert(r.name);
    for (const auto& s : dfa.states) {
        if (s.isAccepting) assert(names.count(s.tokenName) == 1);
    }
    cout << " [PASS] all accepting DFA states carry valid token names." << endl;
}

int main() {
    cout << "====================================================" << endl;
    cout << "        LEXICAL ANALYZER GENERATOR - STEP 2 TESTS   " << endl;
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

    testClosureAndMove();
    testSubsetConstructionSmall();
    testSubsetConstructionFullRules(rulesPath);

    cout << "\nALL STEP 2 TESTS PASSED SUCCESSFULLY!" << endl;
    cout << "====================================================" << endl;
    return 0;
}
