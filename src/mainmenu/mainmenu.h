#pragma once
#include <vector>
#include <string>

class MainMenu {
public:
    // Gibt 1..N zurück (entsprechend Menüpunkt), oder 0 wenn abgebrochen (ESC/q)
    static int show();

private:
    enum class Key { Up, Down, Enter, Escape, Other };

    static void clearScreen();
    static void render(const std::vector<std::string>& items, int selected);

    static Key readKey();

    // Terminal raw mode helper
    class RawMode {
    public:
        RawMode();
        ~RawMode();
        RawMode(const RawMode&) = delete;
        RawMode& operator=(const RawMode&) = delete;
    private:
        bool enabled = false;
        struct termios* oldState = nullptr;
    };
};
