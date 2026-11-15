#include "../include/LexicalAnalyzer.h"
#include <sstream>

LexicalAnalyzer::LexicalAnalyzer(const DFA& minimizedDfa, const string& sourceCode, ostream* errStream)
    : dfa(minimizedDfa), input(sourceCode), cursor(0), currentLine(1), currentColumn(1), errorStream(errStream) {
}

void LexicalAnalyzer::setSourceCode(const string& sourceCode) {
    input = sourceCode;
    cursor = 0;
    currentLine = 1;
    currentColumn = 1;
    symbolTable.clear();
    errorLog.clear();
}

void LexicalAnalyzer::skipWhitespace() {
    while (cursor < input.size()) {
        char c = input[cursor];
        if (c == ' ' || c == '\t' || c == '\r') {
            cursor++;
            currentColumn++;
        } else if (c == '\n') {
            cursor++;
            currentLine++;
            currentColumn = 1;
        } else {
            break;
        }
    }
}

void LexicalAnalyzer::recoverFromError(char badChar, int line, int col) {
    ostringstream oss;
    oss << "Lexical Error at line " << line << ", column " << col
        << ": Unrecognized character '";
    if (badChar >= 32 && badChar <= 126) {
        oss << badChar;
    } else {
        oss << "\\x" << hex << (static_cast<int>(badChar) & 0xFF);
    }
    oss << "'";

    string errMsg = oss.str();
    errorLog.push_back(errMsg);
    if (errorStream) {
        *errorStream << errMsg << "\n";
    }
}

Token LexicalAnalyzer::getNextToken() {
    while (true) {
        skipWhitespace();

        if (cursor >= input.size()) {
            return Token("EOF", "", currentLine, currentColumn);
        }

        int startLine = currentLine;
        int startCol = currentColumn;
        size_t startPos = cursor;

        int state = dfa.startStateId;
        int lastAcceptingState = DFA::NO_STATE;
        size_t lastAcceptingPos = startPos;
        int lastAcceptingLine = currentLine;
        int lastAcceptingCol = currentColumn;

        size_t cur = startPos;
        int line = currentLine;
        int col = currentColumn;

        // Maximal Munch: scan as far forward as possible
        while (cur < input.size()) {
            char c = input[cur];
            int nextState = dfa.getNextState(state, c);
            if (nextState == DFA::NO_STATE) {
                break;
            }
            state = nextState;
            cur++;
            if (c == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }

            if (dfa.isAccepting(state)) {
                lastAcceptingState = state;
                lastAcceptingPos = cur;
                lastAcceptingLine = line;
                lastAcceptingCol = col;
            }
        }

        // Check if an accepting prefix was matched (must consume >= 1 character)
        if (lastAcceptingState != DFA::NO_STATE && lastAcceptingPos > startPos) {
            cursor = lastAcceptingPos;
            currentLine = lastAcceptingLine;
            currentColumn = lastAcceptingCol;

            string lexeme = input.substr(startPos, lastAcceptingPos - startPos);
            string tokenClass = dfa.getAcceptingToken(lastAcceptingState);

            // Record identifiers in the Symbol Table
            if (tokenClass == "id") {
                symbolTable.insert(lexeme, "id", startLine);
            }

            return Token(tokenClass, lexeme, startLine, startCol);
        }

        // Error recovery: no prefix matched any regular expression
        char badChar = input[startPos];
        recoverFromError(badChar, startLine, startCol);

        // Skip the single bad character and continue looking for tokens
        cursor = startPos + 1;
        if (badChar == '\n') {
            currentLine = startLine + 1;
            currentColumn = 1;
        } else {
            currentLine = startLine;
            currentColumn = startCol + 1;
        }
    }
}

bool LexicalAnalyzer::hasMoreTokens() {
    size_t tempCursor = cursor;
    while (tempCursor < input.size()) {
        char c = input[tempCursor];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            tempCursor++;
        } else {
            return true;
        }
    }
    return false;
}

vector<Token> LexicalAnalyzer::tokenizeAll() {
    vector<Token> tokens;
    while (true) {
        Token t = getNextToken();
        if (t.isEOF()) break;
        tokens.push_back(t);
    }
    return tokens;
}
