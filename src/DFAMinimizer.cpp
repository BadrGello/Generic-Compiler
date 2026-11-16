#include "../include/DFAMinimizer.h"
#include <map>
#include <queue>
#include <vector>
#include <tuple>

using namespace std;

DFA DFAMinimizer::minimize(const DFA& dfa) {
    DFA result;
    if (dfa.states.empty()) return result;

    const int n = static_cast<int>(dfa.states.size());
    vector<char> symbols(dfa.alphabet.begin(), dfa.alphabet.end());

    //! 1. Keep only states reachable from the start state

    vector<bool> reachable(n, false);
    {
        queue<int> q;
        q.push(dfa.startStateId);
        reachable[dfa.startStateId] = true;
        while (!q.empty()) {
            int cur = q.front();
            q.pop();
            for (const auto& kv : dfa.states[cur].transitions) {
                if (!reachable[kv.second]) {
                    reachable[kv.second] = true;
                    q.push(kv.second);
                }
            }
        }
    }

    //! 2. Initial partition: non-accepting | accepting grouped by token class.
    //!    block[s] = block index of state s (-1 for unreachable states)

    vector<int> block(n, -1);
    {
        map<pair<bool, string>, int> initial;
        for (int s = 0; s < n; s++) {
            if (!reachable[s]) continue;
            pair<bool, string> key = {dfa.states[s].isAccepting,
                                      dfa.states[s].isAccepting ? dfa.states[s].tokenName : ""};
            auto it = initial.find(key);
            if (it == initial.end()) {
                int id = static_cast<int>(initial.size());
                it = initial.emplace(key, id).first;
            }
            block[s] = it->second;
        }
    }

    //! 3. Refine until no block splits any more.
    //!    Signature of a state = (its block, block of the target for every symbol).
    //!    Missing transition -> -1 (implicit dead state, which is its own class).

    int numBlocks = 0;
    for (int s = 0; s < n; s++) numBlocks = max(numBlocks, block[s] + 1);

    while (true) {
        map<vector<int>, int> signatureToBlock;      //? signature -> new block index
        vector<int> newBlock(n, -1);                 //? new block index for each state

        for (int s = 0; s < n; s++) {
            if (!reachable[s]) continue;
            vector<int> sig;
            sig.reserve(symbols.size() + 1);
            sig.push_back(block[s]);
            for (char c : symbols) {
                int next = dfa.getNextState(s, c);
                sig.push_back(next == DFA::NO_STATE ? -1 : block[next]);
            }
            auto it = signatureToBlock.find(sig);
            if (it == signatureToBlock.end()) {
                int id = static_cast<int>(signatureToBlock.size());
                it = signatureToBlock.emplace(sig, id).first;
            }
            newBlock[s] = it->second;
        }

        int newCount = static_cast<int>(signatureToBlock.size());
        block = newBlock;
        if (newCount == numBlocks) break; //! stable: no block was split
        numBlocks = newCount;
    }

    //! 4. Build the minimized DFA, numbering states in BFS order from the start state
    //!    (deterministic output; start state becomes state 0).
    vector<int> blockToNew(numBlocks, -1);       //? new state ID for each block (-1 = not yet created)
    vector<int> representative(numBlocks, -1);  //? one original state per block
    for (int s = 0; s < n; s++) {
        if (reachable[s] && representative[block[s]] == -1) representative[block[s]] = s;
    }

    queue<int> q;
    auto assign = [&](int b) {
        if (blockToNew[b] != -1) return;
        blockToNew[b] = result.addState();
        q.push(b);
    };

    assign(block[dfa.startStateId]);
    result.startStateId = blockToNew[block[dfa.startStateId]];

    while (!q.empty()) {
        int b = q.front();
        q.pop();
        const DFAState& rep = dfa.states[representative[b]];
        int newId = blockToNew[b];

        if (rep.isAccepting) result.setAccepting(newId, rep.tokenName, rep.priority);

        for (char c : symbols) { // sorted symbols -> deterministic numbering
            int next = dfa.getNextState(rep.id, c);
            if (next == DFA::NO_STATE) continue;
            int nb = block[next];
            assign(nb);
            result.addTransition(newId, c, blockToNew[nb]);
        }
    }

    result.alphabet.insert(dfa.alphabet.begin(), dfa.alphabet.end());
    return result;
}
