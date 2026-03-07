#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <climits>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include <sstream>
#include <format>
#include <iostream>
#include <memory>
#include <regex>

#include "config.h"

#ifndef HOST_NAME_MAX
#if defined(_POSIX_HOST_NAME_MAX)
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#else
#define HOST_NAME_MAX 255
#endif
#endif

Config config{};

void execute_command(const std::vector<std::string> &args) {
    const pid_t process = fork();
    if (process == -1) {
        perror("fork failed");
        return;
    }
    if (process == 0) {
        signal(SIGINT, SIG_DFL);

        std::vector<char *> argsToPass;
        for (const auto &arg: args) {
            argsToPass.push_back(const_cast<char *>(arg.data()));
        }
        argsToPass.push_back(nullptr);

        execvp(args[0].c_str(), argsToPass.data());
        perror("Command execution failed");
        _exit(1);
    }
    int status;
    waitpid(process, &status, 0);
}

std::vector<std::string> tokenise(const std::string& str, const char& delim) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delim);

    while (end != std::string::npos) {
        if (std::string token = str.substr(start, end - start); !token.empty()) tokens.push_back(token);
        start = end + 1;
        end = str.find(delim, start);
    }

    if (std::string final_token = str.substr(start); !final_token.empty()) tokens.push_back(final_token);

    return tokens;
}

std::vector<std::string> split_to_args(const std::string& command) {
    std::vector<std::string> args;
    std::string current;
    bool in_quotes = false;

    for (int i = 0; i < command.length(); i++) {
        char c = command[i];

        if (c == '\\') {
            if (i + 1 < command.length()) {
                current += command[++i];
            }
            continue;
        }

        if (c == '"') {
            in_quotes = !in_quotes;
            continue;
        }

        if (std::isspace(c) && !in_quotes) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
            continue;
        }
        current += c;
    }
    if (!current.empty()) {
        args.push_back(current);
    }

    return args;
}

bool handle_commands(const std::unique_ptr<char, void(*)(void*)> &currentCMD) {
    if (!currentCMD) return false;
    for (const std::vector<std::string> commands = tokenise(currentCMD.get(), ';'); const auto &command: commands) {
        std::vector<std::string> split_command = split_to_args(command);

        if (split_command.empty()) {
            continue;
        }

        if (split_command[0] == "cd") {
            config.cd(split_command);
        } else if (split_command[0] == "exit") {
            return true;
        } else {
            execute_command(split_command);
        }
    }
    return false;
}

char* command_generator(const char* text, const int state) {
    static std::vector<std::string> matches;
    static size_t match_index;

    if (state == 0) {
        matches.clear();
        match_index = 0;
        const std::string prefix(text);

        for (const auto& cmd : config.get_commands()) {
            if (cmd.size() >= prefix.size() && cmd.compare(0, prefix.size(), prefix) == 0) {
                matches.push_back(cmd);
            }
        }
        std::sort(matches.begin(), matches.end());
    }
    if (match_index < matches.size()) {
        return strdup(matches[match_index++].c_str());
    }
    return nullptr;
}

char** complete(const char* text, const int start, int end) {
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    }
    return nullptr;
}

std::unique_ptr<char, void(*)(void*)> get_input() {
    std::string prompt;
    const std::string home_path = config.get_home_path();
    std::string current_dir = config.get_current_directory();
    if (!home_path.empty() && current_dir.starts_with(home_path)) {
        prompt = std::format("{}@{}:~{}$ ", config.get_username(), config.get_hostname(),
                             current_dir.c_str() + home_path.length());
    } else {
        prompt = std::format("{}@{}:{}$ ", config.get_username(), config.get_hostname(),
                             current_dir);
    }
    std::unique_ptr<char, void(*)(void*)> current_cmd(readline(prompt.c_str()), std::free);

    if (current_cmd == nullptr) {
        return current_cmd;
    }

    if (*current_cmd) {
        add_history(current_cmd.get());
        stifle_history(1000);
    }
    return current_cmd;
}

int main(int argc, char *argv[]) {
    rl_basic_word_break_characters = " \t\n\"\\'`@$><=;|&{(";
    rl_completer_quote_characters = "\"\'";
    rl_filename_quote_characters = " \t\n\\\"'@<>=|&()";
    rl_attempted_completion_function = complete;

    while (true) {
        // keep the shell running until the exit command is entered

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
