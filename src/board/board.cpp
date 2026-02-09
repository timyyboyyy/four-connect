#include "board.h"

#include <iostream>
#include <string>

using namespace std;

Board::Board() { reset(); }

void Board::reset() {
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            grid[r][c] = ' ';
}

bool Board::isValidColumn(int col) const {
    return (col >= 0 && col < COLS && grid[0][col] == ' ');
}

int Board::dropDisc(int col, char symbol) {
    if (!isValidColumn(col)) return -1;
    for (int r = ROWS - 1; r >= 0; --r) {
        if (grid[r][col] == ' ') {
            grid[r][col] = symbol;
            return r;
        }
    }
    return -1;
}

bool Board::isFull() const {
    for (int c = 0; c < COLS; ++c)
        if (grid[0][c] == ' ') return false;
    return true;
}

bool Board::checkWin(char s) const {
    // horizontal
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c <= COLS - 4; ++c) {
            if (grid[r][c] == s && grid[r][c + 1] == s && grid[r][c + 2] == s && grid[r][c + 3] == s)
                return true;
        }
    }
    // vertical
    for (int c = 0; c < COLS; ++c) {
        for (int r = 0; r <= ROWS - 4; ++r) {
            if (grid[r][c] == s && grid[r + 1][c] == s && grid[r + 2][c] == s && grid[r + 3][c] == s)
                return true;
        }
    }
    // diag down-right
    for (int r = 0; r <= ROWS - 4; ++r) {
        for (int c = 0; c <= COLS - 4; ++c) {
            if (grid[r][c] == s && grid[r + 1][c + 1] == s && grid[r + 2][c + 2] == s && grid[r + 3][c + 3] == s)
                return true;
        }
    }
    // diag down-left
    for (int r = 0; r <= ROWS - 4; ++r) {
        for (int c = 3; c < COLS; ++c) {
            if (grid[r][c] == s && grid[r + 1][c - 1] == s && grid[r + 2][c - 2] == s && grid[r + 3][c - 3] == s)
                return true;
        }
    }
    return false;
}

void Board::printPretty(char p1Symbol, char p2Symbol) const {
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
