#include "../include/RuleParser.h"
#include "../include/NFABuilder.h"
#include "../include/DFABuilder.h"
#include "../include/DFAMinimizer.h"
#include "../include/LexicalAnalyzer.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>

using namespace std;

// Helper to construct minimized DFA from rules file
DFA buildMinimizedDFA(const string& rulesPath) {
    RuleParser parser;
    vector<TokenRule> rules = parser.parseFile(rulesPath);
    assert(!rules.empty() && "Failed to parse rules file or rules are empty");

    NFA nfa = NFABuilder::buildCombinedNFA(rules);
    DFA dfa = DFABuilder::buildFromNFA(nfa);
    return DFAMinimizer::minimize(dfa);
}

void testSymbolTable() {
    cout << "\n--- TEST 1: SYMBOL TABLE OPERATIONS ---" << endl;
    SymbolTable table;

    assert(table.insert("x", "id", 1) == true);
    assert(table.insert("y", "id", 1) == true);
    assert(table.insert("x", "id", 2) == false); // Duplicate

    assert(table.size() == 2);
    assert(table.contains("x"));
    assert(table.contains("y"));
    assert(!table.contains("z"));

    const SymbolEntry* entryX = table.get("x");
    assert(entryX != nullptr);
    assert(entryX->firstLine == 1);
    assert(entryX->occurrences == 2);

    cout << " [PASS] Symbol table inserts, duplicate counting, and lookups verified." << endl;
}

void testMaximalMunchAndDisambiguation(const DFA& dfa) {
    cout << "\n--- TEST 2: MAXIMAL MUNCH & DISAMBIGUATION ---" << endl;

    // String with no spaces between operators and numbers: "pass!=10"
    string code = "pass!=10";
    LexicalAnalyzer lexer(dfa, code);

    Token t1 = lexer.getNextToken();
    assert(t1.tokenClass == "id" && t1.attributeValue == "pass");

    Token t2 = lexer.getNextToken();
    assert(t2.tokenClass == "relop" && t2.attributeValue == "!=");

    Token t3 = lexer.getNextToken();
    assert(t3.tokenClass == "num" && t3.attributeValue == "10");

    Token t4 = lexer.getNextToken();
    assert(t4.isEOF());

    // Keyword vs identifier: "if" is keyword, "iff" is id
    lexer.setSourceCode("if iff");
    Token k = lexer.getNextToken();
    assert(k.tokenClass == "if" && k.attributeValue == "if");
    Token id = lexer.getNextToken();
    assert(id.tokenClass == "id" && id.attributeValue == "iff");

    // Operator disambiguation: "==" vs "="
    lexer.setSourceCode("== ==");
    Token op1 = lexer.getNextToken();
    assert(op1.tokenClass == "relop" && op1.attributeValue == "==");
    Token op2 = lexer.getNextToken();
    assert(op2.tokenClass == "relop" && op2.attributeValue == "==");

    // Number formats: "123", "123.45", "123.45E6"
    lexer.setSourceCode("123 123.45 123.45E6");
    Token n1 = lexer.getNextToken();
    assert(n1.tokenClass == "num" && n1.attributeValue == "123");
    Token n2 = lexer.getNextToken();
    assert(n2.tokenClass == "num" && n2.attributeValue == "123.45");
    Token n3 = lexer.getNextToken();
    assert(n3.tokenClass == "num" && n3.attributeValue == "123.45E6");

    cout << " [PASS] Longest prefix matching (Maximal Munch) and tie-breaking verified." << endl;
}

void testErrorRecovery(const DFA& dfa) {
    cout << "\n--- TEST 3: ERROR RECOVERY ROUTINE ---" << endl;

    // Code with illegal characters: "@" and "$"
    string codeWithErrors = "int @count = $10 ;";
    ostringstream errCapture;
    LexicalAnalyzer lexer(dfa, codeWithErrors, &errCapture);

    vector<Token> tokens = lexer.tokenizeAll();

    // Errors should have been captured
    assert(lexer.hasErrors());
    assert(lexer.getErrors().size() == 2);

    // The valid tokens surrounding the errors should be extracted intact
    vector<string> expectedClasses = {"int", "id", "assign", "num", ";"};
    vector<string> expectedLexemes = {"int", "count", "=", "10", ";"};

    assert(tokens.size() == expectedClasses.size());
    for (size_t i = 0; i < tokens.size(); i++) {
        assert(tokens[i].tokenClass == expectedClasses[i]);
        assert(tokens[i].attributeValue == expectedLexemes[i]);
    }

    cout << " [PASS] Error recovery logged 2 errors and successfully scanned remaining tokens:" << endl;
    for (const auto& err : lexer.getErrors()) {
        cout << "        " << err << endl;
    }
}

void testSampleProgramFromPdf(const DFA& dfa) {
    cout << "\n--- TEST 4: SAMPLE TEST PROGRAM ---" << endl;

    string program =
        "int sum , count , pass , mnt; while (pass !=10)\n"
        "{\n"
        "pass = pass + 1 ;\n"
        "}";

    LexicalAnalyzer lexer(dfa, program);
    vector<Token> tokens = lexer.tokenizeAll();

    // Expected token classes as documented on Page 3 of the PDF
    vector<string> expectedClasses = {
        "int", "id", ",", "id", ",", "id", ",", "id", ";",
        "while", "(", "id", "relop", "num", ")",
        "{",
        "id", "assign", "id", "addop", "num", ";",
        "}"
    };

    assert(tokens.size() == expectedClasses.size());
    cout << "Token Stream Output:" << endl;
    for (size_t i = 0; i < tokens.size(); i++) {
        cout << "  " << tokens[i].tokenClass << " (\"" << tokens[i].attributeValue << "\")" << endl;
        assert(tokens[i].tokenClass == expectedClasses[i]);
    }

    // Verify Symbol Table has all identifiers
    const SymbolTable& st = lexer.getSymbolTable();
    assert(st.contains("sum"));
    assert(st.contains("count"));
    assert(st.contains("pass"));
    assert(st.contains("mnt"));
    assert(st.size() == 4); // exactly 4 unique identifiers: sum, count, pass, mnt

    // "pass" occurs 4 times
    assert(st.get("pass")->occurrences == 4);

    cout << "\nResultant Symbol Table:" << endl;
    st.print(cout);

    cout << " [PASS] Sample program produced the exact expected token stream and symbol table." << endl;
}

int main() {
    cout << "====================================================" << endl;
    cout << "        LEXICAL ANALYZER GENERATOR - STEP 4 TESTS   " << endl;
    cout << "====================================================" << endl;

    string rulesPath = "tests/lexical_rules.txt";
    ifstream check(rulesPath);
    if (!check.is_open()) {
        rulesPath = "lexical_rules.txt";
    }

    DFA minimalDfa = buildMinimizedDFA(rulesPath);

    testSymbolTable();
    testMaximalMunchAndDisambiguation(minimalDfa);
    testErrorRecovery(minimalDfa);
    testSampleProgramFromPdf(minimalDfa);

    cout << "\nALL STEP 4 TESTS PASSED SUCCESSFULLY!" << endl;
    cout << "====================================================" << endl;
    return 0;
}
