#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <string>
#include <vector>
#include <map>
#include <set>

struct TrieNode {
    std::map<char, int> children;
    int failLink;
    std::vector<std::string> output;
    TrieNode() : failLink(0) {}
};

class AhoCorasick {
private:
    std::vector<TrieNode> trie;
    void buildTrie(const std::vector<std::string>& keywords);
    void buildFailureLinks();

public:
    AhoCorasick(const std::vector<std::string>& keywords);
    std::set<std::string> findKeywords(const std::string& text);
    std::vector<std::pair<int, std::string>> searchWithPositions(const std::string& text);
};

#endif