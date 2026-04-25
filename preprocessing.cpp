#include "preprocessing.h"
#include <iostream>
#include <algorithm>
#include <cctype>

using namespace std;

string preprocessMessage(const string& rawMessage) {
    string s = rawMessage;
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    string result = "";
    for (char c : s) if (isalnum(c) || isspace(c)) result += c;
    cout << "  [PREPROCESS] Cleaned: " << result << endl;
    return result;
}