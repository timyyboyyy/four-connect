#pragma once

#include <string>

class TestSuite {
private:
    int passed = 0;
    int failed = 0;

    void assertTrue(bool cond, const std::string &msg);

public:
    void run();
};
