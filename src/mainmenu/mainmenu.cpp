#include "mainmenu.h"
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <cstring>

#include <clocale>
#include <cwchar>

using std::cout;
using std::string;
using std::vector;

static termios g_old{};
static bool g_hasOld = false;

namespace {

size_t termColsUtf8(const std::string& s) {
    std::mbstate_t st{};
    const char* p = s.c_str();
    size_t cols = 0;

    while (*p) {
        wchar_t wc = 0;
        size_t n = std::mbrtowc(&wc, p, MB_CUR_MAX, &st);

        // Fallback bei kaputter Sequenz: 1 Spalte pro Byte
        if (n == (size_t)-1 || n == (size_t)-2 || n == 0) {
            ++cols;
            ++p;
            std::memset(&st, 0, sizeof(st));
            continue;
        }

        int w = ::wcwidth(wc);
        cols += (w < 0 ? 1 : (size_t)w);
        p += n;
    }
    return cols;
}

std::string padRightCols(const std::string& s, size_t targetCols) {
    size_t cols = termColsUtf8(s);
    if (cols >= targetCols) return s;
    return s + std::string(targetCols - cols, ' ');
}

} // namespace

MainMenu::RawMode::RawMode() {
    termios raw{};
    if (tcgetattr(STDIN_FILENO, &g_old) == 0) {
        g_hasOld = true;
        raw = g_old;

        // Raw-ish mode (no echo, no canonical line buffering)
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN]  = 0; // non-blocking read with timeout via select
        raw.c_cc[VTIME] = 0;

        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
            enabled = true;
        }
    }
}

MainMenu::RawMode::~RawMode() {
    if (enabled && g_hasOld) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_old);
    }
}

void MainMenu::clearScreen() {
    // Clear + home (no colors)
    cout << "\033[2J\033[H";
}

void MainMenu::render(const vector<string>& items, int selected) {
    // Locale einmal setzen, damit mbrtowc/wcwidth korrekt arbeiten
    static bool localeSet = false;
    if (!localeSet) {
        std::setlocale(LC_CTYPE, "");
        localeSet = true;
    }

    clearScreen();

    const char* logo[] = {
        "  ██╗  ██╗     ██████╗ ███████╗██╗    ██╗██╗███╗   ██╗███╗   ██╗████████╗",
        "  ██║  ██║    ██╔════╝ ██╔════╝██║    ██║██║████╗  ██║████╗  ██║╚══██╔══╝",
        "  ███████║    ██║  ███╗█████╗  ██║ █╗ ██║██║██╔██╗ ██║██╔██╗ ██║   ██║   ",
        "  ╚════██║    ██║   ██║██╔══╝  ██║███╗██║██║██║╚██╗██║██║╚██╗██║   ██║   ",
        "       ██║    ╚██████╔╝███████╗╚███╔███╔╝██║██║ ╚████║██║ ╚████║   ██║   ",
        "       ╚═╝     ╚═════╝ ╚══════╝ ╚══╝╚══╝ ╚═╝╚═╝  ╚═══╝╚═╝  ╚═══╝   ╚═╝   ",
    };

    cout << "\n";
    for (auto* line : logo) cout << line << "\n";
    cout << "\n";

    // Box Layout mit fester Breite, damit es nicht bei langen Menüeinträgen zerfetzt wird
    constexpr size_t CONTENT_WIDTH = 50;

    cout << "  ┌────────────────────────────────────────────────────┐\n";
    {
        const string header = "Navigation:  ↑/↓  |  Enter = OK  |  ESC/q = Zurück";
        cout << "  │ " << padRightCols(header, CONTENT_WIDTH) << " │\n";
    }
    cout << "  ├────────────────────────────────────────────────────┤\n";

    for (int i = 0; i < (int)items.size(); ++i) {
        if (i == selected) {
            string line = "> [" + items[i] + "]";
            cout << "  │ " << padRightCols(line, CONTENT_WIDTH) << " │\n";
        } else {
            string line = "  " + items[i];
            cout << "  │ " << padRightCols(line, CONTENT_WIDTH) << " │\n";
        }
    }

    cout << "  └────────────────────────────────────────────────────┘\n";
    cout.flush();
}

MainMenu::Key MainMenu::readKey() {
    // select() mit kleinem Timeout, damit wir nicht busy-waiten
    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);

    timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 200000; // 200ms

    int rv = select(STDIN_FILENO + 1, &set, nullptr, nullptr, &tv);
    if (rv <= 0) return Key::Other;

    char c = 0;
    if (read(STDIN_FILENO, &c, 1) != 1) return Key::Other;

    if (c == '\n' || c == '\r') return Key::Enter;

    if (c == 27) { // ESC oder Escape-Sequence
        // Prüfe ob Arrow-Key Sequenz kommt: ESC [ A/B
        char seq[2]{0, 0};
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return Key::Escape;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return Key::Escape;

        if (seq[0] == '[') {
            if (seq[1] == 'A') return Key::Up;
            if (seq[1] == 'B') return Key::Down;
        }
        return Key::Escape;
    }

    if (c == 'q' || c == 'Q') return Key::Escape;

    return Key::Other;
}

int MainMenu::show() {
    RawMode rm; // aktiviert raw mode, wird am Ende automatisch zurückgesetzt

    vector<string> items = {
        "Neues Spiel starten",
        "Spiel aus Logdatei wiederholen",
        "Tests ausführen",
        "Statistiken anzeigen",
        "Beenden"
    };

    int selected = 0;
    render(items, selected);

    while (true) {
        Key k = readKey();
        switch (k) {
            case Key::Up:
                selected = (selected - 1 + (int)items.size()) % (int)items.size();
                render(items, selected);
                break;
            case Key::Down:
                selected = (selected + 1) % (int)items.size();
                render(items, selected);
                break;
            case Key::Enter:
                return selected + 1;   // 1..N
            case Key::Escape:
                return 0;              // abgebrochen
            case Key::Other:
                break;
        }
    }
}
