#include "util.h"

#include <cctype>
#include <iostream>
#include <limits>

using namespace std;

void clearInputLine() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool isNumber(const string& s) {
    if (s.empty()) return false;
    for (char ch : s) {
        if (!isdigit(static_cast<unsigned char>(ch))) return false;
    }
    return true;
}

void pressEnterToContinue() {
    cout << "Weiter mit Enter...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}
