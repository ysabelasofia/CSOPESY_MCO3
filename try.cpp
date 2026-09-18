#include <iostream>
#include <string>

int main() {
    std::cout << "Welcome to CSOPESY!\n\n";
    std::cout << "Group developer:\n";
    std::cout << "Austria, Ma. Alexandria\n";
    std::cout << "De Leon, Sofia Ysabela\n";
    std::cout << "Guererro, Laura Mae\n";
    std::cout << "Patricio, Anne Beatriz\n\n";
    std::cout << "Version date: 2026-09-18\n\n";
    
    bool running = true;
    while (running)
    {
        std::string command_prompt;
        std::cout << "Command > ";
        std::cin >> command_prompt;

        if (command_prompt != "help" && command_prompt != "set_text" && command_prompt != "exit")
        {
            if (command_prompt == "start_marquee" || command_prompt == "stop_marquee" || command_prompt == "set_speed")
            {
                std::cout << "[feature to be implemented]\n\n";
                // di ko alam if need pa lagyan yung para sa set_speed na input lolol
            }
            else
            {
                std::cout << "Invalid command input. Type 'help' for a list of commands.\n\n";
            }
        }

        if (command_prompt == "help")
        {
            std::cout << "help - displays the commands and its description\n";
            std::cout << "start_marquee - starts the marquee \"animation\"\n";
            std::cout << "stop_marquee - stops the marquee \"animation\"\n";
            std::cout << "set_text - accepts a text input and displays it as a marquee\n";
            std::cout << "set_speed - set the marqee animation refresh in milliseconds\n";
            std::cout << "exit - terminates the console\n\n";
        }

        if (command_prompt == "set_text")
        {
            // di ko sure if ganto dapat ha lol
            std::string text_input;
            std::cin.ignore();
            std::getline(std::cin, text_input);
            std::cout << "Text saved for marquee: " << text_input << "\n\n";
        }

        if (command_prompt == "exit")
        {
            std::cout << "Terminating console...\n";
            running = false;
        }
    }

    return 0;
}
