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
        fprintf(stderr, "Process failed with error code %i.\n", WEXITSTATUS(status));
    }
}

std::vector<std::string> split_string(const std::string &givenString, const std::string &delim) {
    std::vector<std::string> returnVector;
    size_t start = 0;
    size_t end = givenString.find(delim);
    while (end != std::string::npos) {
        returnVector.push_back(givenString.substr(start, end - start));
        start = end + delim.length();
        end = givenString.find(delim, start);
    }
    returnVector.push_back(givenString.substr(start));
    return returnVector;
}

std::vector<std::string> split_whitespace(const std::string &givenString) {
    std::vector<std::string> returnVector;
    std::stringstream ss(givenString);
    std::string word;
    while (ss >> word) {
        returnVector.push_back(word);
    }
    return returnVector;
}

void handle_commands(const char *currentCMD, Config *config) {
    for (const std::vector<std::string> commands = split_string(currentCMD, ";"); const auto &command: commands) {
        std::vector<std::string> splitCommand = split_whitespace(command);

        if (splitCommand.empty()) {
            continue;
        }

        if (strcmp(splitCommand[0].c_str(), "cd") == 0) {
            config->cd(splitCommand);
        } else if (strcmp(splitCommand[0].c_str(), "exit") == 0) {
            exit(EXIT_SUCCESS);
        } else {
            execute_command(splitCommand);
        }
    }
}

char *get_input(const Config *config) {
    std::string prompt;
    if (config->get_home_path() != nullptr && config->get_current_directory().starts_with(config->get_home_path())) {
        prompt = std::format("{}@{}:~{}$ ", config->get_username(), config->get_hostname(),
                             config->get_current_directory().c_str() + strlen(config->get_home_path()));
    } else {
        prompt = std::format("{}@{}:{}$ ", config->get_username(), config->get_hostname(),
                             config->get_current_directory());
    }
    char *currentCMD = readline(prompt.c_str());

    if (currentCMD == nullptr) {
        return nullptr;
    }

    if (*currentCMD) {
        add_history(currentCMD);
    }
    return currentCMD;
}

int main(int argc, char *argv[]) {
    Config config{};

    while (true) {
        // keep the shell running until the exit command is entered

        char *currentCMD = get_input(&config);

        if (currentCMD == nullptr) {
            printf("\n");
            break;
        }

        handle_commands(currentCMD, &config);

        free(currentCMD);
    }

    return 0;
}
