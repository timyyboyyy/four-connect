#include "gamelogger.h"

#include <fstream>
#include <sstream>

using namespace std;

bool GameLogger::save(const string &filename,
                      const vector<Move> &moves,
                      const string players[2],
                      const char symbols[2],
                      int result) {
    ofstream out(filename);
    if (!out) return false;

    out << "CONNECT4_LOG_V2\n";
    out << "PLAYER0:" << players[0] << "\n";
    out << "PLAYER1:" << players[1] << "\n";
    out << "SYMBOL0:" << symbols[0] << "\n";
    out << "SYMBOL1:" << symbols[1] << "\n";
    out << "RESULT:" << result << "\n";

    for (const auto &m : moves) {
        out << "MOVE;" << m.playerIndex << ";" << m.col << ";" << m.moveTimeMs << "\n";
    }
    return true;
}

bool GameLogger::load(const string &filename,
                      vector<Move> &moves,
                      string players[2],
                      char symbols[2],
                      int &result) {
    ifstream in(filename);
    if (!in) return false;

    moves.clear();
    players[0] = "Spieler 1";
    players[1] = "Spieler 2";
    symbols[0] = 'X';
    symbols[1] = 'O';
    result = -1;

    string header;
    if (!getline(in, header)) return false;

    bool v2 = (header == "CONNECT4_LOG_V2");
    bool v1 = (header == "CONNECT4_LOG_V1");
    if (!v1 && !v2) return false;

    string line;
    while (getline(in, line)) {
        if (line.rfind("PLAYER0:", 0) == 0) players[0] = line.substr(8);
        else if (line.rfind("PLAYER1:", 0) == 0) players[1] = line.substr(8);
        else if (line.rfind("SYMBOL0:", 0) == 0) symbols[0] = (line.size() > 8 ? line[8] : 'X');
        else if (line.rfind("SYMBOL1:", 0) == 0) symbols[1] = (line.size() > 8 ? line[8] : 'O');
        else if (line.rfind("RESULT:", 0) == 0) result = stoi(line.substr(7));
        else if (line.rfind("MOVE;", 0) == 0) {
            stringstream ss(line);
            string tok;
            Move m{0, 0, 0};

            getline(ss, tok, ';'); // MOVE
            getline(ss, tok, ';'); m.playerIndex = stoi(tok);
            getline(ss, tok, ';'); m.col = stoi(tok);
            if (v2) {
                getline(ss, tok, ';');
                m.moveTimeMs = tok.empty() ? 0 : stoll(tok);
            } else {
                m.moveTimeMs = 0; // v1 hatte keine Zeiten
            }
            moves.push_back(m);
        }
    }
    return true;
}
