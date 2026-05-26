//
// Created by aven on 26/05/2026.
//

#include "Shell.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/wait.h>

#include "parser.h"

std::string Shell::colour_code(COLOUR colour) {
    std::string colour_code;
    switch (colour) {
        case RED:
            colour_code = "31";
            break;
        case GREEN:
            colour_code = "32";
            break;
        case BLUE:
            colour_code = "34";
            break;
        case DEFAULT:
            colour_code = "39";
            break;
    }
    return std::format("\x1b[{}m", colour_code);
}

std::string Shell::colour_code(RGBColour colour) {
    return std::format("\033[38;2;<{}>;<{}>;<{}>m", colour.r, colour.g, colour.b);
}

void Shell::execute_command(const std::vector<std::string> &args) {
    const pid_t process = fork();
    if (process == -1) {
        perror("fork failed");
        return;
    }
    if (process == 0) {
        signal(SIGINT, SIG_DFL);

        std::vector<char *> argsToPass;
        for (const auto &arg: args) {
            argsToPass.push_back(const_cast<char *>(arg.c_str()));
        }
        argsToPass.push_back(nullptr);

        execvp(args[0].c_str(), argsToPass.data());
        perror("Command execution failed");
        _exit(1);
    }
    int status;
    waitpid(process, &status, 0);
}

bool Shell::handle_commands(const std::unique_ptr<char, void(*)(void *)> &currentCMD) {
    if (!currentCMD) return false;
    Parser parse((currentCMD.get()));
    auto commands = parse.tokenise();

    if (std::ranges::any_of(commands, [](const auto& cmd) {
        return !cmd.empty() && cmd[0] == "exit";
    })) {
        return true;
    }

    for (const auto &command: commands) {

        if (command.empty()) {
            continue;
        }

        if (command[0] == "cd") {
            Config::cd(command);
        } else if (command[0] == "export") {
            Config::export_env(command);
        } else {
            execute_command(command);
        }
    }
    return false;
}

char *Shell::command_generator(const char *text, int state) {
    static std::vector<std::string> matches;
    static size_t match_index;

    if (state == 0) {
        matches.clear();
        match_index = 0;
        const std::string prefix(text);

        for (const auto& cmd : Config::get_commands()) {
            if (cmd.size() >= prefix.size() && cmd.compare(0, prefix.size(), prefix) == 0) {
                matches.push_back(cmd);
            }
        }
        std::ranges::sort(matches);
    }
    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    }
    return nullptr;
}

char **Shell::complete(const char *text, int start, int end) {
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }
    return nullptr;
}

std::unique_ptr<char, void(*)(void *)> Shell::get_input() {
    std::string prompt;
    const std::string home_path = Config::get_home_path();
    std::string current_dir = Config::get_current_directory();
    if (!home_path.empty() && current_dir.starts_with(home_path)) {
        prompt = std::format("{}{}@{}{}:{}~{}{}$ ", colour_code(GREEN), Config::get_username(), Config::get_hostname(), colour_code(DEFAULT), colour_code(GREEN),
                             current_dir.c_str() + home_path.length(), colour_code(DEFAULT));
    } else {
        prompt = std::format("{}{}@{}{}:{}{}{}$ ", colour_code(GREEN), Config::get_username(), Config::get_hostname(), colour_code(DEFAULT), colour_code(GREEN),
                             current_dir, colour_code(DEFAULT));
    }
    std::unique_ptr<char, void(*)(void*)> current_cmd(readline(prompt.c_str()), std::free);

    if (current_cmd == nullptr) {
        return current_cmd;
    }

    if (*current_cmd) {
        add_history(current_cmd.get());
    }
    return current_cmd;
}

int Shell::loop() {
    rl_basic_word_break_characters = " \t\n\"\\'`@$><=;|&{(";
    rl_completer_quote_characters = "\"\'";
    rl_filename_quote_characters = " \t\n\\\"'@<>=|&()";
    rl_attempted_completion_function = complete;
    stifle_history(1000);

    Config::init();

    while (true) { // keep the shell running until the exit command is entered

        std::unique_ptr<char, void(*)(void*)> current_cmd = get_input();

        if (current_cmd == nullptr) {
            std::cout << std::endl;
            break;
        }

        if (handle_commands(current_cmd)) {
            break;
        }
    }

    return 0;
}
