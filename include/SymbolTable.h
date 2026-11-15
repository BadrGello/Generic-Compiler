#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;

// An entry in the Symbol Table for an identifier
struct SymbolEntry {
    string name;       // Identifier name/lexeme
    string tokenClass; // Token type ("id")
    int firstLine = 1; // First line where the identifier appeared
    int occurrences = 1;
};

// Symbol Table for managing identifiers
class SymbolTable {
private:
    unordered_map<string, SymbolEntry> table;
    vector<string> insertionOrder;

public:
    SymbolTable() = default;

    // Inserts an identifier. Returns true if newly inserted, false if already present (increments count)
    bool insert(const string& name, const string& tokenClass = "id", int line = 1);

    // Checks whether an identifier is present
    bool contains(const string& name) const;

    // Look up an entry (nullptr if not found)
    const SymbolEntry* get(const string& name) const;

    // Returns all entries in the order they were first inserted
    vector<SymbolEntry> getAllEntries() const;

    // Returns the total number of unique identifiers
    size_t size() const { return table.size(); }

    // Clears the symbol table
    void clear();

    // Prints the symbol table in tabular format
    void print(ostream& out) const;
};
