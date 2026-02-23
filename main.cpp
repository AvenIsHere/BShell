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

        std::vector<char*> argsToPass;
        for (int i = 0; i < args.size(); i++) {
            argsToPass.push_back(const_cast<std::vector<char *>::value_type>(args[i].data()));
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

typedef struct config {
    const char* homePath;
    std::string currentDirectory;
    std::string username;
    std::string hostname;
    std::string pipeDelim;
}Config;

Config init() {
    const char* homePath = getenv("HOME");

    char currentDirectoryTemp[PATH_MAX];
    if (getcwd(currentDirectoryTemp, PATH_MAX) == nullptr) {
        perror("Could not find current directory");
        exit(1);
    }
    std::string currentDirectory = currentDirectoryTemp;

    const char* usernameTemp = getenv("USER");
    std::string username;
    if (usernameTemp == nullptr) {
        username = "unknown";
    }
    else {
        username = usernameTemp;
    }

    const char* pipeDelimTemp = getenv("PIPE_DELIM");
    std::string pipeDelim;
    if (pipeDelimTemp == nullptr) {
        pipeDelim = "|";
    }
    else {
        pipeDelim = pipeDelimTemp;
    }

    signal(SIGINT, SIG_IGN);

    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    const std::string hostnameStr = hostname;

    Config config = {homePath, currentDirectory, username, hostnameStr, pipeDelim};
    return config;
}

void cd(const std::vector<std::string>& givenCommand, Config* config) {
    if (givenCommand.size()<2) {
        if (config->homePath != nullptr) {
            chdir(config->homePath);
        }
    }
    else if (givenCommand.size()>2) {
        fprintf(stderr, "Too many arguments.\n");
    }
    else {
        errno = 0;
        char dir[PATH_MAX];
        if (givenCommand[1][0] == '~' && config->homePath != nullptr) {
            snprintf(dir, sizeof(dir), "%s%s", config->homePath, givenCommand[1].c_str() + 1);
        }
        else {
            snprintf(dir, sizeof(dir), "%s", givenCommand[1].c_str());
        }
        if (chdir(dir) == -1) {
            perror("cd failed");
        }
    }
    char cwdTemp[PATH_MAX];
    getcwd(cwdTemp, PATH_MAX);
    config->currentDirectory = cwdTemp;
}

std::vector<std::string> split_string(const std::string& givenString, const std::string& delim) {
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

std::vector<std::string> split_whitespace(const std::string& givenString) {
    std::vector<std::string> returnVector;
    std::stringstream ss(givenString);
    std::string word;
    while (ss >> word) {
        returnVector.push_back(word);
    }
    return returnVector;
}

void handle_commands(const char *currentCMD, Config *config) {
    for (const std::vector<std::string> commands = split_string(currentCMD, ";"); const auto & command : commands) {
        std::vector<std::string> splitCommand = split_whitespace(command);

        if (splitCommand.empty()) {
            continue;
        }

        if (strcmp(splitCommand[0].c_str(), "cd") == 0) {
            cd(splitCommand, config);
        }
        else if (strcmp(splitCommand[0].c_str(), "exit") == 0) {
            exit(EXIT_SUCCESS);
        }
        else {
            execute_command(splitCommand);
        }
    }
}

char* get_input(const Config *config) {
    std::string prompt;
    if (config->homePath != nullptr && config->currentDirectory.starts_with(config->homePath)) {
        prompt = std::format("{}@{}:~{}$ ", config->username, config->hostname, config->currentDirectory.c_str() + strlen(config->homePath));
    }
    else {
        prompt = std::format("{}@{}:{}$ ", config->username, config->hostname, config->currentDirectory);
    }
    char* currentCMD = readline(prompt.c_str());

    if (currentCMD == nullptr) {
        return nullptr;
    }

    if (*currentCMD) {
        add_history(currentCMD);
    }
    return currentCMD;
}

int main(int argc, char *argv[]){

    Config config = init();

    while (true){ // keep the shell running until the exit command is entered

        char* currentCMD = get_input(&config);

        if (currentCMD == nullptr) {
            printf("\n");
            break;
        }

        handle_commands(currentCMD, &config);

        free(currentCMD);
    }

    return 0;
}