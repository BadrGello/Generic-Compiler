#include "../include/RuleParser.h"
#include "../include/NFABuilder.h"
#include "../include/DFABuilder.h"
#include "../include/DFAMinimizer.h"
#include "../include/LexicalAnalyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char* argv[]) {
    // Usage: ./lexer_generator [rules_file] [input_program] [output_tokens] [output_table]
    string rulesPath = (argc > 1) ? argv[1] : "tests/lexical_rules.txt";
    string programPath = (argc > 2) ? argv[2] : "tests/sample_program.java";
    string outputTokensPath = (argc > 3) ? argv[3] : "output_tokens.txt";
    string outputTablePath = (argc > 4) ? argv[4] : "minimal_dfa_table.txt";

    // Handle relative path fallbacks when executed directly inside build/
    ifstream testRules(rulesPath);
    if (!testRules.is_open()) {
        rulesPath = "lexical_rules.txt";
    }
    ifstream testProg(programPath);
    if (!testProg.is_open()) {
        programPath = "sample_program.java";
    }

    cout << "========================================================\n";
    cout << "       LEXICAL ANALYZER GENERATOR                       \n";
    cout << "========================================================\n";
    cout << "1. Parsing lexical rules from: " << rulesPath << "\n";

    RuleParser parser;
    vector<TokenRule> rules = parser.parseFile(rulesPath);
    if (rules.empty()) {
        cerr << "Error: No token rules loaded from " << rulesPath << ". Exiting.\n";
        return 1;
    }
    cout << "   -> Loaded " << rules.size() << " token rules.\n";

    cout << "2. Constructing Combined NFA (Thompson's)...\n";
    NFA nfa = NFABuilder::buildCombinedNFA(rules);
    cout << "   -> Combined NFA states: " << nfa.numStates() << "\n";

    cout << "3. Converting NFA to DFA (Subset Construction)...\n";
    DFA dfa = DFABuilder::buildFromNFA(nfa);
    cout << "   -> DFA states: " << dfa.numStates() << "\n";

    cout << "4. Minimizing DFA (Partition Refinement)...\n";
    DFA minimalDfa = DFAMinimizer::minimize(dfa);
    cout << "   -> Minimal DFA states: " << minimalDfa.numStates() << "\n";

    cout << "5. Emitting minimal DFA transition table to: " << outputTablePath << "\n";
    if (!minimalDfa.saveTransitionTableText(outputTablePath)) {
        cerr << "Warning: Could not save transition table to " << outputTablePath << "\n";
    } else {
        cout << "   -> Transition table saved successfully.\n";
    }

    // Also export CSV format
    string csvPath = "minimal_dfa_table.csv";
    size_t dotPos = outputTablePath.find_last_of('.');
    if (dotPos != string::npos) {
        csvPath = outputTablePath.substr(0, dotPos) + ".csv";
    }
    minimalDfa.saveTransitionTableCSV(csvPath);

    cout << "6. Reading input program from: " << programPath << "\n";
    ifstream progFile(programPath);
    if (!progFile.is_open()) {
        cerr << "Error: Could not open program file: " << programPath << "\n";
        return 1;
    }
    stringstream buffer;
    buffer << progFile.rdbuf();
    string sourceCode = buffer.str();

    cout << "7. Running Lexical Analyzer Simulation...\n";
    LexicalAnalyzer lexer(minimalDfa, sourceCode);
    vector<Token> tokens = lexer.tokenizeAll();

    cout << "8. Emitting recognized token classes to: " << outputTokensPath << "\n";
    ofstream tokensOut(outputTokensPath);
    if (!tokensOut.is_open()) {
        cerr << "Error: Could not open output tokens file: " << outputTokensPath << "\n";
        return 1;
    }

    for (const auto& token : tokens) {
        tokensOut << token.tokenClass << "\n";
    }
    tokensOut.close();
    cout << "   -> Emitted " << tokens.size() << " tokens (one per line).\n";

    // Display Symbol Table
    cout << "\n";
    lexer.getSymbolTable().print(cout);

    if (lexer.hasErrors()) {
        cout << "\nWarnings/Errors encountered during scanning (" << lexer.getErrors().size() << "):\n";
        for (const auto& err : lexer.getErrors()) {
            cout << "  " << err << "\n";
        }
    } else {
        cout << "\nScanning completed with 0 lexical errors.\n";
    }

    cout << "========================================================\n";
    cout << "GENERATION & ANALYSIS COMPLETED SUCCESSFULLY\n";
    cout << "========================================================\n";

    return 0;
}
