
#include "aho_corasick.h"
#include <queue>

using namespace std;

AhoCorasick::AhoCorasick(const vector<string>& keywords) {
    buildTrie(keywords);
    buildFailureLinks();
}

void AhoCorasick::buildTrie(const vector<string>& keywords) {
    trie.clear();
    trie.emplace_back(); 
    for (const string& word : keywords) {
        int cur = 0;
        for (char c : word) {
            if (trie[cur].children.find(c) == trie[cur].children.end()) {
                int newIndex = (int)trie.size();
                trie.emplace_back();
                trie[cur].children[c] = newIndex;
            }
            cur = trie[cur].children[c];
        }
        trie[cur].output.push_back(word);
    }
}

void AhoCorasick::buildFailureLinks() {
    queue<int> q;
    for (auto const& item : trie[0].children) {
        trie[item.second].failLink = 0;
        q.push(item.second);
    }
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (auto const& item : trie[u].children) {
            char c = item.first;
            int v = item.second;
            int f = trie[u].failLink;
            while (f > 0 && trie[f].children.find(c) == trie[f].children.end()) 
                f = trie[f].failLink;
            trie[v].failLink = (trie[f].children.count(c)) ? trie[f].children[c] : 0;
            for (const string& out : trie[trie[v].failLink].output)
                trie[v].output.push_back(out);
            q.push(v);
        }
    }
}

set<string> AhoCorasick::findKeywords(const string& text) {
    set<string> found;
    int cur = 0;
    for (char c : text) {
        while (cur > 0 && trie[cur].children.find(c) == trie[cur].children.end()) 
            cur = trie[cur].failLink;
        if (trie[cur].children.count(c)) cur = trie[cur].children[c];
        for (const string& word : trie[cur].output) found.insert(word);
    }
    return found;
}

vector<pair<int, string>> AhoCorasick::searchWithPositions(const string& text) {
    vector<pair<int, string>> results;
    int cur = 0;
    for (int i = 0; i < (int)text.size(); i++) {
        char c = text[i];
        while (cur > 0 && trie[cur].children.find(c) == trie[cur].children.end()) 
            cur = trie[cur].failLink;
        if (trie[cur].children.count(c)) cur = trie[cur].children[c];
        for (const string& word : trie[cur].output)
            results.push_back({i - (int)word.size() + 1, word});
    }
    return results;
}