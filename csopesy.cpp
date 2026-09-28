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
int body_top_row = 0; // 1-based screen row where the scrolling area (below the marquee) starts
//constexpr int INPUT_ROW = 24; // row where user input will be displayed
//constexpr int MESSAGE_ROW = 26;

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

short get_console_width() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return static_cast<short>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
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

void marquee2(std::string& text) {
    if (text.empty()) return;

    char first = text[0]; //gets front charac
    text.erase(0, 1);
    text += first; //moves og first charac to the end
}

// marquee text state struct
struct MarqueeState {
    std::mutex m;
    std::string text;
};

void run_marquee(MarqueeState& state, std::atomic<int>& refresh_speed, std::atomic<bool>& running_flag, short marquee_row) {
    std::string display_text;
    std::string last_seen;
    int position = 0; 

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
                position = 0;
            }
        }

        if (display_text.empty()) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(refresh_speed.load())
            );
            continue;
        }

        #ifdef _WIN32
                short console_width = get_console_width();
        #else
                short console_width = 80;
        #endif

        int text_len = static_cast<int>(display_text.length());

        // ayos visible part of text to console width
        int start_col = std::max(0, position);
        int text_offset = start_col - position; // chars to skip when text starts off-screen left
        int visible_len = std::min(text_len - text_offset, static_cast<int>(console_width) - 1 - start_col);

        {
            std::lock_guard<std::mutex> lock(console_mutex);
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

            // whole frame goes out in ONE write so typing can't land between "save cursor" and "restore cursor"
            std::string frame = "\033" "7"; // save cursor
            frame += "\033[" + std::to_string(marquee_row - csbi.srWindow.Top + 1) + ";1H"; // go to marquee row
            frame += "\033[K";
            if (visible_len > 0 && text_offset < text_len) {
                frame += std::string(start_col, ' ') + display_text.substr(text_offset, visible_len);
            }
            frame += "\033" "8"; // restore cursor
            std::cout << frame << std::flush;
        }

        // move one direction only 
        --position;
        if (position < -text_len) {
            position = console_width;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(refresh_speed.load())
        );
    }

    {
        std::lock_guard<std::mutex> lock(console_mutex);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        std::string frame = "\033" "7";
        frame += "\033[" + std::to_string(marquee_row - csbi.srWindow.Top + 1) + ";1H";
        frame += "\033[K";
        frame += "\033" "8";
        std::cout << frame << std::flush;
    }
}

void print_message(const std::string& msg) {
    std::lock_guard<std::mutex> lock(console_mutex);
    if (body_top_row > 0 && msg.find('\n') != std::string::npos) {
        // multi-line output (help): start from the top of the scrolling area so its first lines don't scroll off
        std::cout << "\033[" << body_top_row << ";1H\033[J";
    }
    std::cout << msg << "\n" << std::flush;
}

int main() {
#ifdef _WIN32
    enable_ansi();
#endif
    std::cout << "\033[2J\033[H" << std::flush; // clear screen, home cursor
    print_banner();

    std::cout << COLOR_YELLOW << "Welcome to CSOPESY!\n\n"
            << "Group Developers:\n";
    std::cout << COLOR_GREEN   << "Austria, Ma. Alexandria\n"
            << "De Leon, Sofia Ysabela\n"
            << "Guererro, Laura Mae\n"
            << "Patricio, Anne Beatriz\n\n";
    std::cout << COLOR_YELLOW << "Version date: 2026-09-27\n\n" << RESET_COLOR;

    std::cout << "\n\n\n\033[3A" << std::flush; // make sure 3 rows exist below (scrolls if window is short), then go back up

    marquee_row = get_cursor_pos().Y;

    bool running = true;
    //int refresh_speed = 500; // can change 
    std::atomic<int> refresh_speed{500};
    MarqueeState marquee_text;
    marquee_text.text = "Default Text";
    std::atomic<bool> running_marquee{false};
    std::thread marquee_thread;
    
    std::string line;
    line.reserve(128); // pre-set memory

#ifdef _WIN32
    // only everything BELOW the marquee row may scroll, so the marquee row stays put
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int top_row = marquee_row - csbi.srWindow.Top + 2;              // 1-based screen row right under the marquee
        int bottom_row = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;  // last visible row
        body_top_row = top_row;
        std::cout << "\033[" << top_row << ";" << bottom_row << "r" << std::flush;
    }
#endif
    set_cursor_pos(0, marquee_row + 2);

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
            // gagana parin even if may whitespace unahan and dulo
            // args empty lang pag only trailing whitespace after command
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

                running_marquee.store(true);
                print_message("Current speed: " + std::to_string(refresh_speed) + " ms");

                marquee_thread = std::thread(run_marquee, std::ref(marquee_text), std::ref(refresh_speed), std::ref(running_marquee), marquee_row);
            }

            //std::cout << "Starting marquee with text: " << text_input << "\n\n";
            //std::cout << "[feature to be implemented]\n\n";
            // di ko alam if need pa lagyan yung para sa set_speed na input lolol
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
            
            //std::cout << "[feature to be implemented]\n\n";
            // di ko alam if need pa lagyan yung para sa set_speed na input lolol
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