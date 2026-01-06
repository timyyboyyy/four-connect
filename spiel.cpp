#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <limits>
#include <ctime>
#include <cctype>
#include <chrono>

#if __has_include(<filesystem>)
  #include <filesystem>
  namespace fs = std::filesystem;
  #define HAS_FILESYSTEM 1
#else
  #define HAS_FILESYSTEM 0
#endif

using namespace std;

static constexpr int ROWS = 6;
static constexpr int COLS = 7;

// ===================== Utility =====================

static void clearInputLine() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

static bool isNumber(const string& s) {
    if (s.empty()) return false;
    for (char ch : s) {
        if (!isdigit(static_cast<unsigned char>(ch))) return false;
    }
    return true;
}

static void pressEnterToContinue() {
    cout << "Weiter mit Enter...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ===================== Data Types =====================

struct Move {
    int playerIndex;         // 0 oder 1
    int col;                 // 0..6
    long long moveTimeMs;    // optional: Zeit pro Zug in ms (für spätere Auswertung)
};

// ===================== Board =====================

class Board {
private:
    char grid[ROWS][COLS];

public:
    Board() { reset(); }

    void reset() {
        for (int r = 0; r < ROWS; ++r)
            for (int c = 0; c < COLS; ++c)
                grid[r][c] = ' ';
    }

    bool isValidColumn(int col) const {
        return (col >= 0 && col < COLS && grid[0][col] == ' ');
    }

    int dropDisc(int col, char symbol) {
        if (!isValidColumn(col)) return -1;
        for (int r = ROWS - 1; r >= 0; --r) {
            if (grid[r][col] == ' ') {
                grid[r][col] = symbol;
                return r;
            }
        }
        return -1;
    }

    bool isFull() const {
        for (int c = 0; c < COLS; ++c)
            if (grid[0][c] == ' ') return false;
        return true;
    }

    bool checkWin(char s) const {
        // horizontal
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c <= COLS - 4; ++c) {
                if (grid[r][c] == s && grid[r][c+1] == s && grid[r][c+2] == s && grid[r][c+3] == s)
                    return true;
            }
        }
        // vertical
        for (int c = 0; c < COLS; ++c) {
            for (int r = 0; r <= ROWS - 4; ++r) {
                if (grid[r][c] == s && grid[r+1][c] == s && grid[r+2][c] == s && grid[r+3][c] == s)
                    return true;
            }
        }
        // diag down-right
        for (int r = 0; r <= ROWS - 4; ++r) {
            for (int c = 0; c <= COLS - 4; ++c) {
                if (grid[r][c] == s && grid[r+1][c+1] == s && grid[r+2][c+2] == s && grid[r+3][c+3] == s)
                    return true;
            }
        }
        // diag down-left
        for (int r = 0; r <= ROWS - 4; ++r) {
            for (int c = 3; c < COLS; ++c) {
                if (grid[r][c] == s && grid[r+1][c-1] == s && grid[r+2][c-2] == s && grid[r+3][c-3] == s)
                    return true;
            }
        }
        return false;
    }

    // "Bildschirmmaske": hübsches Brett (Box Drawing)
    void printPretty(char p1Symbol, char p2Symbol) const {
        // Legende: Wir zeigen statt X/O optional ●/○ im Print.
        auto render = [&](char cell) -> string {
            if (cell == p1Symbol) return "●";  // Spieler 1
            if (cell == p2Symbol) return "○";  // Spieler 2
            return " ";
        };

        cout << "\n    1   2   3   4   5   6   7\n";
        cout << "  ╔═══╦═══╦═══╦═══╦═══╦═══╦═══╗\n";
        for (int r = 0; r < ROWS; ++r) {
            cout << "  ║";
            for (int c = 0; c < COLS; ++c) {
                cout << " " << render(grid[r][c]) << " ║";
            }
            cout << "\n";
            if (r != ROWS - 1)
                cout << "  ╠═══╬═══╬═══╬═══╬═══╬═══╬═══╣\n";
        }
        cout << "  ╚═══╩═══╩═══╩═══╩═══╩═══╩═══╝\n\n";
    }
};

// ===================== Logger =====================

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
    static bool save(const string &filename,
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

    static bool load(const string &filename,
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
                Move m{0,0,0};

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
};

// ===================== TestSuite =====================

class TestSuite {
private:
    int passed = 0;
    int failed = 0;

    void assertTrue(bool cond, const string &msg) {
        if (cond) ++passed;
        else { ++failed; cout << "[FEHLER] " << msg << "\n"; }
    }

public:
    void run() {
        cout << "\n=== TestSuite ===\n";
        passed = failed = 0;

        // Test 1: Stein fällt nach unten
        {
            Board b;
            int row = b.dropDisc(0, 'X');
            assertTrue(row == ROWS - 1, "dropDisc: Stein sollte unten landen.");
        }

        // Test 2: Volle Spalte
        {
            Board b;
            for (int i = 0; i < ROWS; ++i) assertTrue(b.dropDisc(2, 'O') != -1, "Spalte sollte auffüllbar sein.");
            assertTrue(b.dropDisc(2, 'O') == -1, "dropDisc: volle Spalte muss -1 liefern.");
        }

        // Test 3: Horizontal Win
        {
            Board b;
            b.dropDisc(0, 'X'); b.dropDisc(1, 'X'); b.dropDisc(2, 'X'); b.dropDisc(3, 'X');
            assertTrue(b.checkWin('X'), "checkWin: horizontaler Sieg.");
        }

        // Test 4: Vertikal Win
        {
            Board b;
            b.dropDisc(5, 'O'); b.dropDisc(5, 'O'); b.dropDisc(5, 'O'); b.dropDisc(5, 'O');
            assertTrue(b.checkWin('O'), "checkWin: vertikaler Sieg.");
        }

        // Test 5: Diagonal \ Win
        {
            Board b;
            b.dropDisc(0, 'X');
            b.dropDisc(1, 'O'); b.dropDisc(1, 'X');
            b.dropDisc(2, 'O'); b.dropDisc(2, 'O'); b.dropDisc(2, 'X');
            b.dropDisc(3, 'O'); b.dropDisc(3, 'O'); b.dropDisc(3, 'O'); b.dropDisc(3, 'X');
            assertTrue(b.checkWin('X'), "checkWin: diagonal \\ Sieg.");
        }

        // Test 6: isFull
        {
            Board b;
            char s[2] = {'X','O'};
            int idx = 0;
            for (int c = 0; c < COLS; ++c)
                for (int r = 0; r < ROWS; ++r) { b.dropDisc(c, s[idx]); idx = 1 - idx; }
            assertTrue(b.isFull(), "isFull: Brett sollte voll sein.");
        }

        cout << "Bestanden: " << passed << " | Fehlgeschlagen: " << failed << "\n";
        cout << "=================\n\n";
    }
};

// ===================== GameEngine =====================

class GameEngine {
private:
    Board board;
    string players[2];
    char symbols[2] = {'X', 'O'};
    vector<Move> moves;
    int currentPlayer = 0;

    static string makeLogFileName() {
        time_t now = time(nullptr);
        tm *lt = localtime(&now);
        char buf[64];
        strftime(buf, sizeof(buf), "connect4_%Y%m%d_%H%M%S.log", lt);

        // logs/ Ordner nutzen (wird erstellt)
#if HAS_FILESYSTEM
        try { fs::create_directories("logs"); } catch (...) {}
        return string("logs/") + buf;
#else
        return string(buf);
#endif
    }

    // Rückgabe:
    // -1 => Abbruch
    //  0..6 => gültige Spalte
    int readColumnOrAbort(const string &playerName) {
        while (true) {
            cout << playerName << " wählt Spalte (1-7) | 0/q = Abbruch: ";
            string input;
            cin >> input;

            if (input == "0" || input == "q" || input == "Q") return -1;

            if (!isNumber(input)) {
                cout << "Ungültig. Bitte 1-7 oder 0/q.\n";
                continue;
            }

            int col = stoi(input);
            if (col < 1 || col > 7) {
                cout << "Bitte nur 1 bis 7 (oder 0/q).\n";
                continue;
            }
            return col - 1;
        }
    }

public:
    void startNewGame() {
        board.reset();
        moves.clear();
        currentPlayer = 0;

        clearInputLine(); // falls vorher cin >> genutzt wurde
        cout << "\n=== Neues Spiel ===\n";
        cout << "Name Spieler 1 (Symbol X / Anzeige ●): ";
        getline(cin, players[0]);
        if (players[0].empty()) players[0] = "Spieler 1";

        cout << "Name Spieler 2 (Symbol O / Anzeige ○): ";
        getline(cin, players[1]);
        if (players[1].empty()) players[1] = "Spieler 2";

        bool gameOver = false;
        int result = -1; // 0 draw, 1 p0 win, 2 p1 win, -1 none

        while (!gameOver) {
            board.printPretty(symbols[0], symbols[1]);

            const string &pName = players[currentPlayer];
            char sym = symbols[currentPlayer];

            cout << "Am Zug: " << pName << " (" << (currentPlayer == 0 ? "●" : "○") << ")\n";

            auto tStart = chrono::steady_clock::now();
            int col = readColumnOrAbort(pName);
            auto tEnd = chrono::steady_clock::now();
            long long ms = chrono::duration_cast<chrono::milliseconds>(tEnd - tStart).count();

            if (col == -1) {
                cout << "Spiel wurde abgebrochen. Kein Log wird gespeichert.\n\n";
                clearInputLine();
                return; // zurück ins Menü
            }

            if (!board.isValidColumn(col)) {
                cout << "Diese Spalte ist voll. Bitte eine andere Spalte wählen.\n";
                continue;
            }

            if (board.dropDisc(col, sym) == -1) {
                cout << "Interner Fehler beim Setzen. Bitte erneut.\n";
                continue;
            }

            moves.push_back({currentPlayer, col, ms});

            if (board.checkWin(sym)) {
                board.printPretty(symbols[0], symbols[1]);
                cout << "Sieg für " << pName << "!\n";
                result = currentPlayer + 1;
                gameOver = true;
            } else if (board.isFull()) {
                board.printPretty(symbols[0], symbols[1]);
                cout << "Unentschieden (Brett voll).\n";
                result = 0;
                gameOver = true;
            } else {
                currentPlayer = 1 - currentPlayer;
            }
        }

        // speichern
        string fn = makeLogFileName();
        if (GameLogger::save(fn, moves, players, symbols, result)) {
            cout << "Log gespeichert: " << fn << "\n\n";
        } else {
            cout << "Fehler: Log konnte nicht gespeichert werden.\n\n";
        }
    }

    static void replayFromFile(const string &filename) {
        vector<Move> moves;
        string players[2];
        char symbols[2];
        int result = -1;

        if (!GameLogger::load(filename, moves, players, symbols, result)) {
            cout << "Replay fehlgeschlagen: Datei nicht lesbar oder falsches Format.\n\n";
            return;
        }

        cout << "\n=== Replay: " << filename << " ===\n";
        cout << "Spieler 1: " << players[0] << " (●)\n";
        cout << "Spieler 2: " << players[1] << " (○)\n";
        cout << "Hinweis: Enter = nächster Zug, q = abbrechen\n\n";

        clearInputLine();

        Board board;
        int moveNr = 1;

        for (const auto &m : moves) {
            cout << "Zug " << moveNr << ": " << players[m.playerIndex]
                 << " -> Spalte " << (m.col + 1);
            if (m.moveTimeMs > 0) cout << " (" << m.moveTimeMs << " ms)";
            cout << "\n";

            if (board.dropDisc(m.col, symbols[m.playerIndex]) == -1) {
                cout << "Replay abgebrochen: Ungültiger Zug im Log (Spalte voll).\n";
                break;
            }

            board.printPretty(symbols[0], symbols[1]);

            // Schrittweise Wiedergabe
            string line;
            getline(cin, line);
            if (!line.empty() && (line[0] == 'q' || line[0] == 'Q')) {
                cout << "Replay abgebrochen.\n\n";
                return;
            }

            ++moveNr;
        }

        cout << "Replay beendet. Ergebnis: ";
        if (result == 0) cout << "Unentschieden\n\n";
        else if (result == 1) cout << "Sieg " << players[0] << "\n\n";
        else if (result == 2) cout << "Sieg " << players[1] << "\n\n";
        else cout << "Unbekannt\n\n";
    }
};

// ===================== main (Menü) =====================

int main() {
    GameEngine engine;
    TestSuite tests;

    while (true) {
        cout << "==== 4 GEWINNT - SPIELENGINE ====\n";
        cout << "1) Neues Spiel starten\n";
        cout << "2) Spiel aus Logdatei wiederholen\n";
        cout << "3) Tests ausführen\n";
        cout << "4) Beenden\n";
        cout << "Auswahl: ";

        int choice;
        if (!(cin >> choice)) {
            cout << "Ungültige Eingabe.\n";
            clearInputLine();
            continue;
        }

        if (choice == 1) {
            engine.startNewGame();
        } else if (choice == 2) {
            cout << "Dateiname (z.B. logs/connect4_YYYYMMDD_HHMMSS.log): ";
            string fn;
            cin >> fn;
            GameEngine::replayFromFile(fn);
        } else if (choice == 3) {
            tests.run();
        } else if (choice == 4) {
            cout << "Beenden.\n";
            break;
        } else {
            cout << "Bitte 1-4 wählen.\n";
        }
    }

    return 0;
}
