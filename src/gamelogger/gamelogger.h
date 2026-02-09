#pragma once

#include <string>
#include <vector>

#include "move.h"

class GameLogger {
public:
    // Format (V2, enthält moveTimeMs):
    // CONNECT4_LOG_V2
    // PLAYER0:...
    // PLAYER1:...
    // SYMBOL0:X
    // SYMBOL1:O
    // RESULT:0/1/2
    // MOVE;playerIndex;col;moveTimeMs
    static bool save(const std::string &filename,
                     const std::vector<Move> &moves,
                     const std::string players[2],
                     const char symbols[2],
                     int result);

    static bool load(const std::string &filename,
                     std::vector<Move> &moves,
                     std::string players[2],
                     char symbols[2],
                     int &result);
};
