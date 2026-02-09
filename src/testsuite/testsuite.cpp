#include "testsuite.h"

#include <iostream>

#include "board/board.h"
#include "constants.h"

using namespace std;

void TestSuite::assertTrue(bool cond, const string &msg) {
    if (cond) ++passed;
    else { ++failed; cout << "[FEHLER] " << msg << "\n"; }
}

void TestSuite::run() {
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

    // Test 5: Diagonal \\ Win
    {
        Board b;
        b.dropDisc(0, 'X');
        b.dropDisc(1, 'O'); b.dropDisc(1, 'X');
        b.dropDisc(2, 'O'); b.dropDisc(2, 'O'); b.dropDisc(2, 'X');
        b.dropDisc(3, 'O'); b.dropDisc(3, 'O'); b.dropDisc(3, 'O'); b.dropDisc(3, 'X');
        assertTrue(b.checkWin('X'), "checkWin: diagonal \\\\ Sieg.");
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
