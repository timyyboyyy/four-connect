#pragma once

// Ein einzelner Spielzug (für Logging / Replay)
struct Move {
    int playerIndex;         // 0 oder 1
    int col;                 // 0..6
    long long moveTimeMs;    // optional: Zeit pro Zug in ms
};
