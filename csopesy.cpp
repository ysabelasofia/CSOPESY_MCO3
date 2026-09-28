// to compile: g++ -std=c++17 -O2 -o main.exe main.cpp -lpthread

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <charconv>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <vector>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif

// ANSI color codes for CLI text
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_CYAN "\033[0;36m"
#define RESET_COLOR "\033[0m"

std::mutex console_mutex; // to sync console output
short marquee_row = 0;
//constexpr int INPUT_ROW = 24; // row where user input will be displayed
//constexpr int MESSAGE_ROW = 26;

// number of console rows reserved at the top for the animated scene
constexpr int SCENE_H = 10;

// Win32 console colors used by the scene
constexpr WORD ATTR_GRAY   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
constexpr WORD ATTR_WHITE  = ATTR_GRAY | FOREGROUND_INTENSITY;
constexpr WORD ATTR_CYAN   = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
constexpr WORD ATTR_BLUE   = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
constexpr WORD ATTR_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
constexpr WORD ATTR_RED    = FOREGROUND_RED | FOREGROUND_INTENSITY;
constexpr WORD ATTR_SIGN   = ATTR_WHITE | BACKGROUND_BLUE; // white text on blue sign

#ifdef _WIN32
// ANSI escape on Windows
void enable_ansi() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif

#ifdef _WIN32
COORD get_cursor_pos() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.dwCursorPosition; // gives live (X, Y) coordinate
}

void set_cursor_pos(short x, short y) {
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {x, y});
}
#endif

void print_banner() {
    std::cout << COLOR_CYAN <<
R"(+------------------------------------------------------------------+
| __        _______ _     ____ ___  __  __ _____   _____ ___     |
| \ \      / / ____| |   / ___/ _ \|  \/  | ____| |_   _/ _ \    |
|  \ \ /\ / /|  _| | |  | |  | | | | |\/| |  _|     | || | | |   |
|   \ V  V / | |___| |__| |__| |_| | |  | | |___    | || |_| |   |
|    \_/\_/  |_____|_____\____\___/|_|  |_|_____|   |_| \___/    |
|                                                                |
|   ____ ____   ___  ____  _____ ______   __  __  __  ___ _____  |
|  / ___/ ___| / _ \|  _ \| ____/ ___\ \ / / |  \/  |/ _ \___ /  |
| | |   \___ \| | | | |_) |  _| \___ \\ V /  | |\/| | | | ||_ \  |
| | |___ ___) | |_| |  __/| |___ ___) || |   | |  | | |_| |__) | |
|  \____|____/ \___/|_|   |_____|____/ |_|   |_|  |_|\___/____/  |
+------------------------------------------------------------------+
)" << RESET_COLOR << std::flush;
}

// welcome text + developer names (moved into a function so it can be redrawn after the banner is cleared)
void print_welcome() {
    std::cout << COLOR_YELLOW << "Welcome to CSOPESY!\n\n"
            << "Group Developers:\n";
    std::cout << COLOR_GREEN   << "Austria, Ma. Alexandria\n"
            << "De Leon, Sofia Ysabela\n"
            << "Guererro, Laura Mae\n"
            << "Patricio, Anne Beatriz\n\n";
    std::cout << COLOR_YELLOW << "Version date: 2026-09-27\n\n" << RESET_COLOR;
}

// marquee text state struct
struct MarqueeState {
    std::mutex m;
    std::string text;
};


// in-memory grid of characters + colors for one animation frame.
struct Canvas {
    int w;
    std::vector<std::string> ch;
    std::vector<std::vector<WORD>> at;

    explicit Canvas(int width)
        : w(width),
          ch(SCENE_H, std::string(width, ' ')),
          at(SCENE_H, std::vector<WORD>(width, ATTR_GRAY)) {}

    // draw string s at (row, x). spaces are transparent unless opaque == true
    void put(int row, int x, const std::string& s, WORD attr, bool opaque = false) {
        if (row < 0 || row >= SCENE_H) return;
        for (size_t i = 0; i < s.size(); ++i) {
            int col = x + static_cast<int>(i);
            if (col < 0 || col >= w) continue;
            if (!opaque && s[i] == ' ') continue;
            ch[row][col] = s[i];
            at[row][col] = attr;
        }
    }
};


// a boat sailing on animated waves, carrying the text on its sign
void draw_boat(Canvas& cv, const std::string& text, int x, int frame) {
    const int L = static_cast<int>(text.size());
    const int W = L + 8;

    // drifting clouds on the top row (slower than the boat)
    static const std::string sky =
        "      .--.                   .-~~-.                 _  .--.            ";
    for (int c = 0; c < cv.w; ++c) {
        char s = sky[(c + frame / 3) % sky.size()];
        if (s != ' ') cv.put(0, c, std::string(1, s), ATTR_GRAY);
    }

    // waves (drawn first so the hull can sit in them)
    static const std::string wv = "~-_~~-_-~~_-";
    const int n = static_cast<int>(wv.size());
    const WORD wattr[3] = {ATTR_CYAN, ATTR_CYAN, ATTR_BLUE};
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < cv.w; ++c) {
            int i = (((c - frame + r * 3) % n) + n) % n;
            cv.put(SCENE_H - 3 + r, c, std::string(1, wv[i]), wattr[r]);
        }
    }

    // boat bobs up and down
    const int bob = (frame / 5) % 2;
    const int top = 1 + bob;
    const int mid = x + W / 2;

    cv.put(top,     mid, "|>", ATTR_RED);
    cv.put(top + 1, mid, "|",  ATTR_WHITE);

    // cabin / sign
    cv.put(top + 2, x + 2, "." + std::string(L + 2, '-') + ".", ATTR_YELLOW);
    cv.put(top + 3, x + 2, "|", ATTR_YELLOW);
    cv.put(top + 3, x + 3, " " + text + " ", ATTR_SIGN, true);
    cv.put(top + 3, x + L + 5, "|", ATTR_YELLOW);
    cv.put(top + 4, x + 2, "'" + std::string(L + 2, '-') + "'", ATTR_YELLOW);

    // hull with portholes
    std::string hull(W - 2, '_');
    for (int i = 2; i < W - 3; i += 4) hull[i] = 'o';
    cv.put(top + 5, x,     "\\" + hull + "/", ATTR_RED);
    cv.put(top + 6, x + 1, "\\" + std::string(W - 4, '_') + "/", ATTR_RED);
}

void run_marquee(MarqueeState& state, std::atomic<int>& refresh_speed, std::atomic<bool>& running_flag,
                 short marquee_row) {
    std::string display_text;
    std::string last_seen;
    int obj_x = 0;
    int frame = 0;
    int width = 80;
    bool need_reset = true; // restart the object from the right edge

    {
        std::lock_guard<std::mutex> lock(state.m);
        display_text = state.text;
        last_seen = state.text;
    }

    while (running_flag.load()) {
        {
            std::lock_guard<std::mutex> lock(state.m);
            if (state.text != last_seen) {
                display_text = state.text;
                last_seen = state.text;
                need_reset = true;
            }
        }

        if (display_text.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(refresh_speed.load()));
            continue;
        }

        const int L = static_cast<int>(display_text.size());
        const int obj_w = L + 8; // boat width

        {
            std::lock_guard<std::mutex> lock(console_mutex);
            HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(h, &csbi);
            width = csbi.srWindow.Right - csbi.srWindow.Left + 1;

            if (need_reset) {
                obj_x = width;
                need_reset = false;
            }

            Canvas cv(width);
            draw_boat(cv, display_text, obj_x, frame);

            // write straight into the scene rows' cells; this never touches the cursor, so it can't move while typing
            for (int r = 0; r < SCENE_H; ++r) {
                DWORD written = 0;
                COORD pos = {0, static_cast<SHORT>(marquee_row + r)};
                WriteConsoleOutputCharacterA(h, cv.ch[r].c_str(), static_cast<DWORD>(width), pos, &written);
                WriteConsoleOutputAttribute(h, cv.at[r].data(), static_cast<DWORD>(width), pos, &written);
            }
        }

        // move one direction only (right to left)
        --obj_x;
        ++frame;
        if (obj_x < -obj_w) need_reset = true;

        std::this_thread::sleep_for(std::chrono::milliseconds(refresh_speed.load()));
    }

    // clear the scene area
    {
        std::lock_guard<std::mutex> lock(console_mutex);
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(h, &csbi);
        for (int r = 0; r < SCENE_H; ++r) {
            DWORD written = 0;
            COORD pos = {0, static_cast<SHORT>(marquee_row + r)};
            FillConsoleOutputCharacterA(h, ' ', csbi.dwSize.X, pos, &written);
            FillConsoleOutputAttribute(h, ATTR_GRAY, csbi.dwSize.X, pos, &written);
        }
    }
}

void print_message(const std::string& msg) {
    std::lock_guard<std::mutex> lock(console_mutex);
    std::cout << msg << "\n" << std::flush;
}

int main() {
#ifdef _WIN32
    enable_ansi();
#endif
    std::cout << "\033[2J\033[H" << std::flush; // clear screen, home cursor

    marquee_row = get_cursor_pos().Y; // top row of the window (scene goes here once the marquee starts)

    print_banner();

    print_welcome();

    bool running = true;
    std::atomic<int> refresh_speed{100}; // 100 ms per frame looks smoother for a scene; use set_speed to change
    MarqueeState marquee_text;
    marquee_text.text = "Default Text";
    std::atomic<bool> running_marquee{false};
    std::thread marquee_thread;
    bool banner_cleared = false; // banner is wiped (names kept) only the first time the marquee starts

    std::string line;
    line.reserve(128); // pre-set memory

    while (running) {
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\nCommand > " << std::flush;
        }
        if (!std::getline(std::cin, line)) break; // reads input

        std::string_view sv(line);

        size_t start = sv.find_first_not_of(" \t"); // pang hanap ng first char
        if (start == std::string_view::npos) continue; // loop if empty input
        sv.remove_prefix(start);

        size_t cmd_end = sv.find_first_of(" \t");
        std::string_view cmd = sv.substr(0, cmd_end);

        std::string_view args;
        if (cmd_end != std::string_view::npos) {
            size_t args_start = sv.find_first_not_of(" \t", cmd_end);
            if (args_start != std::string_view::npos) {
                args = sv.substr(args_start);
            }
        }

        if (cmd == "help") {
            print_message("help - displays the commands and its description\n"
                          "start_marquee - starts the marquee \"animation\"\n"
                          "stop_marquee - stops the marquee \"animation\"\n"
                          "set_text - accepts a text input and displays it as a marquee\n"
                          "set_speed - set the marquee animation refresh in milliseconds\n"
                          "exit - terminates the console");
        }

        else if (cmd == "set_text") {
            if (args.empty()) {
                print_message("Error: Missing text argument for set_text.");
            } else {
                {
                    std::lock_guard<std::mutex> lock(marquee_text.m);
                    marquee_text.text = std::string(args);
                }
                print_message("Text saved for marquee: " + std::string(args));
            }
        }

        else if (cmd == "set_speed") {
            int speed = 0;
            auto [ptr, ec] = std::from_chars(args.data(), args.data() + args.size(), speed);

            if (ec == std::errc{} && ptr == args.data() + args.size() && speed > 0) {
                refresh_speed = speed;
                print_message("Marquee speed set to: " + std::to_string(refresh_speed) + " ms");
            } else {
                print_message("Invalid speed input. Please enter a positive integer.");
            }
        }

        else if (cmd == "start_marquee") {
            if (running_marquee.load()) {
                print_message("Marquee is already running!");
            } else {
                if (marquee_thread.joinable()) {
                    marquee_thread.join();
                }

                if (!banner_cleared) {
                    // wipe the ASCII banner, reserve the scene rows at the top, redraw the names right under them
                    std::lock_guard<std::mutex> lock(console_mutex);
                    std::cout << "\033[2J\033[H" << std::flush; // clear screen, home cursor
                    CONSOLE_SCREEN_BUFFER_INFO csbi;
                    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
                    marquee_row = csbi.dwCursorPosition.Y; // scene starts at the top of the window
                    int win_top = csbi.srWindow.Top;
                    int win_bottom = csbi.srWindow.Bottom;
                    set_cursor_pos(0, marquee_row + SCENE_H);
                    print_welcome();
                    std::cout << std::flush;
                    COORD after_names = get_cursor_pos();
                    // from now on only the rows below the names scroll
                    std::cout << "\033[" << (after_names.Y - win_top + 1) << ";" << (win_bottom - win_top + 1) << "r" << std::flush;
                    set_cursor_pos(0, after_names.Y);
                    banner_cleared = true;
                }

                running_marquee.store(true);

                marquee_thread = std::thread(run_marquee, std::ref(marquee_text), std::ref(refresh_speed),
                                             std::ref(running_marquee), marquee_row);
            }
        }

        else if (cmd == "stop_marquee") {
            if (running_marquee.load()) {
                running_marquee.store(false);
                if (marquee_thread.joinable()) {
                    marquee_thread.join();
                }
                print_message("Marquee stopped.");
            } else {
                print_message("Marquee is not currently running.");
            }
        }

        else if (cmd == "exit") {
            print_message("Terminating console...");
            if (running_marquee.load()) {
                running_marquee.store(false); // stop marquee thread if running
                if (marquee_thread.joinable()) {
                    marquee_thread.join();
                }
            }
            running = false;
        }

        else {
            print_message("Invalid command input. Type 'help' for a list of commands.");
        }
    }

    std::cout << "\033[r" << std::flush; // release the scroll region
    return 0;
}