#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <climits>
#include <sys/wait.h>
#include <cerrno>
#include <vector>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>
#include <sstream>
#include <format>
#include <iostream>

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
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        std::cerr << "Process failed with error code" << WEXITSTATUS(status) << "." << std::endl;
    }
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

void handle_commands(const char *currentCMD, Config *config) {
    for (const std::vector<std::string> commands = split_string(currentCMD, ";"); const auto &command: commands) {
        std::vector<std::string> split_command = split_whitespace(command);

        if (split_command.empty()) {
            continue;
        }

        if (split_command[0] == "cd") {
            config->cd(split_command);
        } else if (split_command[0] == "exit") {
            exit(EXIT_SUCCESS);
        } else {
            execute_command(split_command);
        }
    }
}

char *get_input(const Config *config) {
    std::string prompt;
    if (config->get_home_path().c_str() != nullptr && config->get_current_directory().starts_with(config->get_home_path())) {
        prompt = std::format("{}@{}:~{}$ ", config->get_username(), config->get_hostname(),
                             config->get_current_directory().c_str() + strlen(config->get_home_path().c_str()));
    } else {
        prompt = std::format("{}@{}:{}$ ", config->get_username(), config->get_hostname(),
                             config->get_current_directory());
    }
    char *current_cmd = readline(prompt.c_str());

    if (current_cmd == nullptr) {
        return nullptr;
    }

    if (*current_cmd) {
        add_history(current_cmd);
    }
    return current_cmd;
}

int main(int argc, char *argv[]) {
    Config config{};

    while (true) {
        // keep the shell running until the exit command is entered

        char *current_cmd = get_input(&config);

        if (current_cmd == nullptr) {
            std::cout << std::endl;
            break;
        }

        handle_commands(current_cmd, &config);

        free(current_cmd);
    }

    return 0;
}
