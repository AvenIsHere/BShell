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

#include "config.h"

#ifndef HOST_NAME_MAX
#if defined(_POSIX_HOST_NAME_MAX)
#define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
#else
#define HOST_NAME_MAX 255
#endif
#endif


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

std::vector<std::string> split_string(const std::string &given_string, const std::string &delim) {
    std::vector<std::string> return_vector;
    size_t start = 0;
    size_t end = given_string.find(delim);
    while (end != std::string::npos) {
        return_vector.push_back(given_string.substr(start, end - start));
        start = end + delim.length();
        end = given_string.find(delim, start);
    }
    return_vector.push_back(given_string.substr(start));
    return return_vector;
}

std::vector<std::string> split_whitespace(const std::string &given_string) {
    std::vector<std::string> returnVector;
    std::stringstream ss(given_string);
    std::string word;
    while (ss >> word) {
        returnVector.push_back(word);
    }
    return returnVector;
}

bool handle_commands(std::unique_ptr<char, void(*)(void*)> currentCMD, Config *config) {
    if (!currentCMD) return false;
    for (const std::vector<std::string> commands = split_string(currentCMD.get(), ";"); const auto &command: commands) {
        std::vector<std::string> split_command = split_whitespace(command);

        if (split_command.empty()) {
            continue;
        }

        if (split_command[0] == "cd") {
            config->cd(split_command);
        } else if (split_command[0] == "exit") {
            return true;
        } else {
            execute_command(split_command);
        }
    }
    return false;
}

std::unique_ptr<char, void(*)(void*)> get_input(const Config *config) {
    std::string prompt;
    const std::string home_path = config->get_home_path();
    std::string current_dir = config->get_current_directory();
    if (!home_path.empty() && current_dir.starts_with(home_path)) {
        prompt = std::format("{}@{}:~{}$ ", config->get_username(), config->get_hostname(),
                             current_dir.c_str() + home_path.length());
    } else {
        prompt = std::format("{}@{}:{}$ ", config->get_username(), config->get_hostname(),
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
    Config config{};

    while (true) {
        // keep the shell running until the exit command is entered

        std::unique_ptr<char, void(*)(void*)> current_cmd = get_input(&config);

        if (current_cmd == nullptr) {
            std::cout << std::endl;
            break;
        }

        if (handle_commands(std::move(current_cmd), &config)) {
            break;
        }
    }

    return 0;
}
