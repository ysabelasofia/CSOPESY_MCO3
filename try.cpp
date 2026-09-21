#include <iostream>
#include <string>
#include <sstream>

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
    while (running)
    {
        std::cout << "Command > ";
        std::string line;
        std::getline(std::cin, line); // read first
        std::stringstream ss(line); // uses stringstream instead to read entire line
        std::string command_prompt;
        ss >> command_prompt; // unang word lang kukunin to get the command ?

        if (command_prompt == "help")
        {
            std::cout << "help - displays the commands and its description\n";
            std::cout << "start_marquee - starts the marquee \"animation\"\n";
            std::cout << "stop_marquee - stops the marquee \"animation\"\n";
            std::cout << "set_text - accepts a text input and displays it as a marquee\n";
            std::cout << "set_speed - set the marqee animation refresh in milliseconds\n";
            std::cout << "exit - terminates the console\n\n";
        }

        else if (command_prompt == "set_text")
        {
            // di ko sure if ganto dapat ha lol
            std::string text_input = "";
            std::getline(ss >> std::ws, text_input); // gets rest of the line, but discards whitespace
            std::cout << "Text saved for marquee: " << text_input << "\n\n";
        }

        else if (command_prompt == "set_speed")
        {
            int speed;
            if (ss >> speed && speed > 0)
            {
                refresh_speed = speed;
                std::cout << "Marquee speed set to: " << refresh_speed << " ms\n\n";
            }
            else
            {
                std::cout << "Invalid speed input. Please enter a positive integer.\n\n";
            }
        }

        else if (command_prompt == "start_marquee")
        {
            std::cout << "[feature to be implemented]\n\n";
            // di ko alam if need pa lagyan yung para sa set_speed na input lolol
        }

        else if (command_prompt == "stop_marquee")
        {
            std::cout << "[feature to be implemented]\n\n";
            // di ko alam if need pa lagyan yung para sa set_speed na input lolol
        }

        else if (command_prompt == "exit")
        {
            std::cout << "Terminating console...\n";
            running = false;
        }

        else
        {
            std::cout << "Invalid command input. Type 'help' for a list of commands.\n\n";
        }
        
    }

    return 0;
}
