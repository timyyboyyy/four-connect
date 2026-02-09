#include <iostream>
#include <string>

#include "gameengine/gameengine.h"
#include "testsuite/testsuite.h"
#include "util/util.h"

using namespace std;

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
