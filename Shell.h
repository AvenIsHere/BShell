//
// Created by aven on 26/05/2026.
//

#ifndef BSHELL_SHELL_H
#define BSHELL_SHELL_H
#include <cstdint>
#include <memory>

#include "config.h"


class Shell {

    Config config;

    enum COLOUR {
        RED,
        GREEN,
        BLUE,
        DEFAULT,
    };

    struct RGBColour {
        uint8_t r, g, b;
    };

    std::string colour_code(RGBColour colour);
    std::string colour_code(COLOUR colour);

public:

    void execute_command(const std::vector<std::string> &args);
    bool handle_commands(const std::unique_ptr<char, void(*)(void*)> &currentCMD);
    static char* command_generator(const char* text, int state);
    static char** complete(const char* text, int start, int end);
    std::unique_ptr<char, void(*)(void*)> get_input();

    int loop();

};


#endif //BSHELL_SHELL_H
