#pragma once

#include <string>

class GameEngine {
public:
    void startNewGame();
    static void replayFromFile(const std::string &filename);
};
