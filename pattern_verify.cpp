#include "pattern_verify.h"
#include <iostream>

void verifyEmergency(const std::string& text, const std::string& pattern) {
    if (text.find(pattern) != std::string::npos) {
        std::cout << "  [VERIFY] Pattern '" << pattern << "' confirmed." << std::endl;
    } else {
        std::cout << "  [VERIFY] Pattern mismatch for '" << pattern << "'." << std::endl;
    }
}