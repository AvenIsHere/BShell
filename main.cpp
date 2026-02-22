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

#ifndef HOST_NAME_MAX
  #if defined(_POSIX_HOST_NAME_MAX)
    #define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
  #else
    #define HOST_NAME_MAX 255
  #endif
#endif

char** vector_to_c(const std::vector<std::string>& givenVector) {
    const auto returnList = static_cast<char**>(malloc(sizeof(char*)*(givenVector.size()+1)));

    for (int i = 0; i < givenVector.size(); i++) {
        returnList[i] = static_cast<char*>(malloc(givenVector[i].length() + 1));
        strcpy(returnList[i], givenVector[i].data());
    }
    returnList[givenVector.size()] = nullptr;

    return returnList;
}


void execute_command(const std::vector<std::string> &args) {
    const pid_t process = fork();
    if (process == -1) {
        perror("fork failed");
        return;
    }
    if (process == 0) {
        signal(SIGINT, SIG_DFL);
        execvp(args[0].c_str(), vector_to_c(args));
        perror("Command execution failed");
        _exit(1);
    }
    int status;
    waitpid(process, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Process failed with error code %i.\n", WEXITSTATUS(status));
    }
}

bool starts_with(const std::string& string, const std::string& prefix) {
    if (prefix.empty()) {
        return true;
    }
    if (prefix.size() > string.size()) {
        return false;
    }
    return strncmp(string.c_str(), prefix.c_str(), prefix.size()) == 0;
}

typedef struct config {
    const char* homePath;
    std::string currentDirectory;
    std::string username;
    char hostname[HOST_NAME_MAX];
    std::string pipeDelim;
}Config;

Config init() {
    const char* homePath = getenv("HOME");

    std::string currentDirectory(PATH_MAX, '\0');
    if (getcwd(&currentDirectory[0], PATH_MAX) == nullptr) {
        perror("Could not find current directory");
        exit(1);
    }

    std::string username = getenv("USER");
    if (username.data() == nullptr) {
        username = "unknown";
    }

    std::string pipeDelim = getenv("PIPE_DELIM");
    if (pipeDelim.data() == nullptr) {
        pipeDelim = "|";
    }

    signal(SIGINT, SIG_IGN);

    Config config = {homePath, currentDirectory, username};
    gethostname(config.hostname, HOST_NAME_MAX);
    config.pipeDelim = pipeDelim;
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
            if (ENOENT == errno) {
                perror("Directory does not exist");
            }
            else {
                perror("cd failed");
            }
        }
    }
    getcwd(config->currentDirectory.data(), PATH_MAX);
}

std::vector<std::string> split_string(std::string givenString, std::string delim) {
    std::vector<std::string> returnVector;
    std::stringstream ss(givenString);
    std::string word;

    while (ss >> word) {
        returnVector.push_back(word);
    }

    return returnVector;
}

void handle_commands(char *currentCMD, Config *config) {
    const std::vector<std::string> commands = split_string(currentCMD, ";");
    for (int k = 0; k < commands.size(); k++) {
        std::vector<std::string> splitCommand = split_string(commands[k], " \t\n");

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

    char prompt[PATH_MAX + HOST_NAME_MAX + 32];
    if (config->homePath != nullptr && starts_with(config->currentDirectory, config->homePath)) {
        snprintf(prompt, sizeof(prompt), "%s@%s:~%s$ ", config->username.c_str(), config->hostname, config->currentDirectory.c_str() + strlen(config->homePath));
    }
    else {
        snprintf(prompt, sizeof(prompt), "%s@%s:%s$ ", config->username.c_str(), config->hostname, config->currentDirectory.c_str());
    }
    char *currentCMD = readline(prompt);

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