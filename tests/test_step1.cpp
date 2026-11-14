#include "../include/RuleParser.h"
#include "../include/NFABuilder.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>
#include <algorithm>

using namespace std;

void testThompsonPrimitives() {
    cout << "\n--- TEST 1: THOMPSON'S PRIMITIVES ---" << endl;

    // 1. Literal 'x'
    {
        NFA nfa;
        NFAFragment f = NFABuilder::createLiteral(nfa, 'x');
        assert(nfa.numStates() == 2);
        assert(nfa.states[f.start].transitions['x'].count(f.accept) == 1);
        cout << " [PASS] createLiteral('x') creates 2 states with 'x' transition." << endl;
    }

    // 2. Lambda (\L)
    {
        NFA nfa;
        NFAFragment f = NFABuilder::createLambda(nfa);
        assert(nfa.numStates() == 2);
        assert(nfa.states[f.start].epsilonTransitions.count(f.accept) == 1);
        cout << " [PASS] createLambda() creates 2 states with epsilon transition." << endl;
    }

    // 3. Concat 'a' . 'b'
    {
        NFA nfa;
        NFAFragment fa = NFABuilder::createLiteral(nfa, 'a');
        NFAFragment fb = NFABuilder::createLiteral(nfa, 'b');
        NFAFragment fc = NFABuilder::createConcat(nfa, fa, fb);
        assert(fc.start == fa.start);
        assert(fc.accept == fb.accept);
        assert(nfa.states[fa.accept].epsilonTransitions.count(fb.start) == 1);
        cout << " [PASS] createConcat('a', 'b') connects fa.accept -> fb.start." << endl;
    }

    // 4. Union 'a' | 'b'
    {
        NFA nfa;
        NFAFragment fa = NFABuilder::createLiteral(nfa, 'a');
        NFAFragment fb = NFABuilder::createLiteral(nfa, 'b');
        NFAFragment fu = NFABuilder::createUnion(nfa, fa, fb);
        // fu has new start branching to fa.start and fb.start
        assert(nfa.states[fu.start].epsilonTransitions.count(fa.start) == 1);
        assert(nfa.states[fu.start].epsilonTransitions.count(fb.start) == 1);
        // fa.accept and fb.accept transition to fu.accept
        assert(nfa.states[fa.accept].epsilonTransitions.count(fu.accept) == 1);
        assert(nfa.states[fb.accept].epsilonTransitions.count(fu.accept) == 1);
        cout << " [PASS] createUnion('a', 'b') correctly branches and joins via epsilon." << endl;
    }

    // 5. Star 'a'*
    {
        NFA nfa;
        NFAFragment fa = NFABuilder::createLiteral(nfa, 'a');
        NFAFragment fs = NFABuilder::createStar(nfa, fa);
        // New start to fa.start
        assert(nfa.states[fs.start].epsilonTransitions.count(fa.start) == 1);
        // fa.accept to new accept
        assert(nfa.states[fa.accept].epsilonTransitions.count(fs.accept) == 1);
        // Repeat loopback: fa.accept to fa.start
        assert(nfa.states[fa.accept].epsilonTransitions.count(fa.start) == 1);
        // Bypass (0 times): fs.start to fs.accept
        assert(nfa.states[fs.start].epsilonTransitions.count(fs.accept) == 1);
        cout << " [PASS] createStar('a') includes loopback and bypass epsilon transitions." << endl;
    }

    // 6. Plus 'a'+
    {
        NFA nfa;
        NFAFragment fa = NFABuilder::createLiteral(nfa, 'a');
        NFAFragment fp = NFABuilder::createPlus(nfa, fa);
        // New start to fa.start
        assert(nfa.states[fp.start].epsilonTransitions.count(fa.start) == 1);
        // fa.accept to new accept
        assert(nfa.states[fa.accept].epsilonTransitions.count(fp.accept) == 1);
        // Repeat loopback: fa.accept to fa.start
        assert(nfa.states[fa.accept].epsilonTransitions.count(fa.start) == 1);
        // NO bypass
        assert(nfa.states[fp.start].epsilonTransitions.count(fp.accept) == 0);
        cout << " [PASS] createPlus('a') includes loopback and NO bypass." << endl;
    }
}

int main(int argc, char* argv[]) {
    cout << "====================================================" << endl;
    cout << "        LEXICAL ANALYZER GENERATOR - STEP 1 TESTS   " << endl;
    cout << "====================================================" << endl;

    // Test individual Thompson primitives first
    testThompsonPrimitives();

    cout << "\n--- TEST 2: COMBINED MASTER NFA FROM RULES ---" << endl;

    string rulesPath = "tests/lexical_rules.txt";
    if (argc > 1) {
        rulesPath = argv[1];
    } else {
        vector<string> candidatePaths = {
            "tests/lexical_rules.txt",
            "../tests/lexical_rules.txt",
            "lexical_rules.txt",
            "../../tests/lexical_rules.txt"
        };
        for (const auto& path : candidatePaths) {
            ifstream f(path);
            if (f.good()) {
                rulesPath = path;
                break;
            }
        }
    }

    RuleParser parser;
    vector<TokenRule> rules = parser.parseFile(rulesPath);
    assert(!rules.empty());
    cout << "Loaded " << rules.size() << " token rules from " << rulesPath << endl;

    // Build the Combined Master NFA
    NFA combinedNfa = NFABuilder::buildCombinedNFA(rules);

    cout << "\nCombined NFA Statistics:" << endl;
    cout << "  Total States: " << combinedNfa.numStates() << endl;
    cout << "  Start State:  " << combinedNfa.startStateId << endl;
    cout << "  Alphabet Size: " << combinedNfa.alphabet.size() << " unique characters" << endl;

    // Display Alphabet
    cout << "  Alphabet: [ ";
    for (char c : combinedNfa.alphabet) {
        if (c >= 32 && c <= 126) cout << c << " ";
        else cout << "\\" << (int)(unsigned char)c << " ";
    }
    cout << "]" << endl;

    // Validate Master Start State transitions
    const NFAState& startState = combinedNfa.states[combinedNfa.startStateId];
    assert(startState.epsilonTransitions.size() == rules.size());
    cout << " [PASS] Master start state (ID " << combinedNfa.startStateId 
         << ") has exactly " << startState.epsilonTransitions.size() 
         << " epsilon transitions (one per token rule)." << endl;

    // Validate Accepting States
    vector<int> acceptingStates = combinedNfa.getAcceptingStates();
    assert(acceptingStates.size() == rules.size());
    cout << " [PASS] Total accepting states count (" << acceptingStates.size() 
         << ") matches rule count (" << rules.size() << ")." << endl;

    // Verify all 18 rules are represented among accepting states with correct priorities
    cout << "\nAccepting States in Combined NFA:" << endl;
    set<string> foundTokens;
    for (int accId : acceptingStates) {
        const NFAState& s = combinedNfa.states[accId];
        assert(s.isAccepting);
        assert(!s.tokenName.empty());
        foundTokens.insert(s.tokenName);
        cout << "  State " << setw(4) << s.id 
             << " -> " << setw(10) << left << s.tokenName 
             << " (Priority " << s.priority << ")" << endl;
    }

    assert(foundTokens.size() == rules.size());
    cout << " [PASS] All " << foundTokens.size() << " distinct tokens have designated accepting states." << endl;

    // Check specific essential tokens
    assert(foundTokens.count("boolean") && foundTokens.count("int") && foundTokens.count("while"));
    assert(foundTokens.count("id") && foundTokens.count("num") && foundTokens.count("relop"));
    assert(foundTokens.count(";") && foundTokens.count("(") && foundTokens.count(")"));
    cout << " [PASS] Keywords, identifiers, numbers, operators, and punctuations all verified." << endl;

    cout << "\nALL STEP 1 TESTS PASSED SUCCESSFULLY!" << endl;
    cout << "====================================================" << endl;
    return 0;
}
