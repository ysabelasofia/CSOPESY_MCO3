// to compile: g++ -std=c++17 -O2 -o try.exe try.cpp -lpthread

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

std::mutex console_mutex; // to sync console output
//constexpr int MARQUEE_ROW = 22; // row where marquee will be displayed, change na lang idk pano sya so that it will be dynamic...
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
#endif

void print_banner() {
    std::cout <<
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
)" << std::flush;
}

void marquee2(std::string& text) {
    if (text.empty()) return;

    char first = text[0]; //gets front charac
    text.erase(0, 1);
    text += first; //moves og first charac to the end
}

constexpr short MARQUEE_ROW = 20;

void run_marquee(std::string marquee_text, std::atomic<int>& refresh_speed, std::atomic<bool>& running_flag) {
    // changed to looping until dlag is false
    while (running_flag.load()) {
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            COORD current_pos = get_cursor_pos();

            //move to marquee row and redraw
            set_cursor_pos(0, MARQUEE_ROW);
            std::cout << "\033[K" << marquee_text << std::flush;

            // put cursor back to where the user is typing/viewing
            set_cursor_pos(current_pos.X, current_pos.Y);
        }
        marquee2(marquee_text);

        int time = 0;
        while (time < refresh_speed.load() && running_flag.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            time += 10;
        }
    }

    // Clean up terminal line when finished or stopped
    {
        std::lock_guard<std::mutex> lock(console_mutex);
        COORD current_pos = get_cursor_pos();
        set_cursor_pos(0, MARQUEE_ROW);
        std::cout << "\033[K" << std::flush;
        set_cursor_pos(current_pos.X, current_pos.Y);
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
    print_banner();

    std::cout << "Welcome to CSOPESY!\n\n";
    std::cout << "Group Developers:\n";
    std::cout << "Austria, Ma. Alexandria\n";
    std::cout << "De Leon, Sofia Ysabela\n";
    std::cout << "Guererro, Laura Mae\n";
    std::cout << "Patricio, Anne Beatriz\n\n";
    std::cout << "Version date: 2026-09-23\n\n\n";

    bool running = true;
    //int refresh_speed = 500; // can change 
    std::atomic<int> refresh_speed{500};
    std::string text_input = "Default Text";
    std::atomic<bool> running_marquee{false};
    std::thread marquee_thread;
    
    std::string line;
    line.reserve(128); // pre-set memory
    set_cursor_pos(0, MARQUEE_ROW + 2);

    while (running) {
        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "\nCommand> " << std::flush;
        }
        if (!std::getline(std::cin, line)) break; // reads input 

        std::string_view sv(line);

        size_t start = sv.find_first_not_of(" \t"); // pang hanap ng first char
        if (start == std::string_view::npos) continue; // loop if empty input
        sv.remove_prefix(start); 

        size_t cmd_end = sv.find_first_of(" \t");
        std::string_view cmd = sv.substr(0, cmd_end);

        std::string_view args = (cmd_end != std::string_view::npos) 
            ? sv.substr(sv.find_first_not_of(" \t", cmd_end)) 
            : std::string_view{};

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
                text_input = std::string(args);
                print_message("Text saved for marquee: " + text_input);
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

                marquee_thread = std::thread(run_marquee, text_input, std::ref(refresh_speed), std::ref(running_marquee));
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

    return 0;
}
