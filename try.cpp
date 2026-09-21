#include <iostream>
#include <string>
#include <string_view>
#include <charconv>
#include <thread>
#include <chrono>

void marquee2(std::string& text)
{
    if (text.empty()) return;

    char first = text[0]; //gets front charac
    text.erase(0, 1);
    text += first; //moves og first charac to the end
}

int main() {
    std::cout << "Welcome to CSOPESY!\n\n";
    std::cout << "Group Developers:\n";
    std::cout << "Austria, Ma. Alexandria\n";
    std::cout << "De Leon, Sofia Ysabela\n";
    std::cout << "Guererro, Laura Mae\n";
    std::cout << "Patricio, Anne Beatriz\n\n";
    std::cout << "Version date: 2026-09-21\n\n";

    bool running = true;
    int refresh_speed = 500; // can change 
    std::string text_input = "Default Text";
    
    std::string line;
    line.reserve(128); // pre-set memory

    while (running) {
        std::cout << "Command> ";
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
            std::cout << "help - displays the commands and its description\n"
                      << "start_marquee - starts the marquee \"animation\"\n"
                      << "stop_marquee - stops the marquee \"animation\"\n"
                      << "set_text - accepts a text input and displays it as a marquee\n"
                      << "set_speed - set the marquee animation refresh in milliseconds\n"
                      << "exit - terminates the console\n\n";
        }

        else if (cmd == "set_text") {
            if (args.empty()) {
                std::cout << "Error: Missing text argument for set_text.\n\n";
            } else {
                text_input = std::string(args);
                std::cout << "Text saved for marquee: " << text_input << "\n\n";
            }
        }

        else if (cmd == "set_speed") {
            int speed = 0;
            auto [ptr, ec] = std::from_chars(args.data(), args.data() + args.size(), speed);
            
            if (ec == std::errc{} && speed > 0) {
                refresh_speed = speed;
                std::cout << "Marquee speed set to: " << refresh_speed << " ms\n\n";
            } else {
                std::cout << "Invalid speed input. Please enter a positive integer.\n\n";
            }
        }

        else if (cmd == "start_marquee") {
            std::string marquee_text = text_input; //make a copy of the text input para hindi mamodify ung og

            for (int i = 0; i < marquee_text.length()*2+1; i++) //scrolls through the text 2 complete times
            {
                std::cout << "\r" << marquee_text << std::flush;

                marquee2(marquee_text);

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(refresh_speed) //controls speed at which the text moves
                );
            }

            std::cout << "\n\n";

            //std::cout << "Starting marquee with text: " << text_input << "\n\n";
            //std::cout << "[feature to be implemented]\n\n";
            // di ko alam if need pa lagyan yung para sa set_speed na input lolol
        }

        else if (cmd == "stop_marquee") {
            std::cout << "[feature to be implemented]\n\n";
            // di ko alam if need pa lagyan yung para sa set_speed na input lolol
        }

        else if (cmd == "exit") {
            std::cout << "Terminating console...\n";
            running = false;
        }

        else {
            std::cout << "Invalid command input. Type 'help' for a list of commands.\n\n";
        }
    }

    return 0;
}
