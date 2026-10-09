//
// Created by aven on 26/05/2026.
//

#ifndef BSHELL_SHELL_H
#define BSHELL_SHELL_H
#include <cstdint>
#include <memory>

#include "config.h"


class Shell {

    enum COLOUR {
        RED,
        GREEN,
        BLUE,
        DEFAULT,
    };

    struct RGBColour {
        uint8_t r, g, b;
    };

    static std::string colour_code(RGBColour colour);
    static std::string colour_code(COLOUR colour);

public:

    static std::string build_path();

    static void execute_command(const std::vector<std::string> &args);
    static bool handle_commands(const std::unique_ptr<char, void(*)(void*)> &currentCMD);

    static char* command_generator(const char* text, int state);
    static char** complete(const char* text, int start, int end);

    static std::unique_ptr<char, void(*)(void*)> get_input();

    static int loop();

};


#endif //BSHELL_SHELL_H
