#include "../include/SymbolTable.h"
#include <iomanip>

bool SymbolTable::insert(const string& name, const string& tokenClass, int line) {
    auto it = table.find(name);
    if (it != table.end()) {
        it->second.occurrences++;
        return false;
    }
    table[name] = SymbolEntry{name, tokenClass, line, 1};
    insertionOrder.push_back(name);
    return true;
}

bool SymbolTable::contains(const string& name) const {
    return table.find(name) != table.end();
}

const SymbolEntry* SymbolTable::get(const string& name) const {
    auto it = table.find(name);
    if (it == table.end()) return nullptr;
    return &(it->second);
}

vector<SymbolEntry> SymbolTable::getAllEntries() const {
    vector<SymbolEntry> entries;
    entries.reserve(insertionOrder.size());
    for (const auto& name : insertionOrder) {
        auto it = table.find(name);
        if (it != table.end()) {
            entries.push_back(it->second);
        }
    }
    return entries;
}

void SymbolTable::clear() {
    table.clear();
    insertionOrder.clear();
}

void SymbolTable::print(ostream& out) const {
    out << "========================================================\n";
    out << "                      SYMBOL TABLE                      \n";
    out << "========================================================\n";
    out << left << setw(20) << "Identifier"
        << setw(15) << "Type"
        << setw(12) << "First Line"
        << setw(12) << "Occurrences" << "\n";
    out << "--------------------------------------------------------\n";
    for (const auto& entry : getAllEntries()) {
        out << left << setw(20) << entry.name
            << setw(15) << entry.tokenClass
            << setw(12) << entry.firstLine
            << setw(12) << entry.occurrences << "\n";
    }
    out << "========================================================\n";
}
