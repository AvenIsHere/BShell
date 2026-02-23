//
// Created by aven on 23/02/2026.
//

#include "config.h"

#include <csignal>
#include <unistd.h>
#include <vector>
#include <bits/local_lim.h>
#include <bits/signum-generic.h>
#include <linux/limits.h>

Config::Config() {
    this->homePath = getenv("HOME");


    char currentDirectoryTemp[PATH_MAX];
    if (getcwd(currentDirectoryTemp, PATH_MAX) == nullptr) {
        perror("Could not find current directory");
        exit(1);
    }
    this->currentDirectory = currentDirectoryTemp;

    const char *usernameTemp = getenv("USER");
    std::string username;
    if (usernameTemp == nullptr) {
        username = "unknown";
    } else {
        username = usernameTemp;
    }
    this->username = username;

    const char *pipeDelimTemp = getenv("PIPE_DELIM");
    std::string pipeDelim;
    if (pipeDelimTemp == nullptr) {
        pipeDelim = "|";
    } else {
        pipeDelim = pipeDelimTemp;
    }
    this->pipeDelim = pipeDelim;

    signal(SIGINT, SIG_IGN);

    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    const std::string hostnameStr = hostname;
    this->hostname = hostnameStr;
}

void Config::cd(const std::vector<std::string> &givenCommand) {
    if (givenCommand.size() < 2) {
        if (homePath != nullptr) {
            chdir(homePath);
        }
    } else if (givenCommand.size() > 2) {
        fprintf(stderr, "Too many arguments.\n");
    } else {
        errno = 0;
        char dir[PATH_MAX];
        if (givenCommand[1][0] == '~' && homePath != nullptr) {
            snprintf(dir, sizeof(dir), "%s%s", homePath, givenCommand[1].c_str() + 1);
        } else {
            snprintf(dir, sizeof(dir), "%s", givenCommand[1].c_str());
        }
        if (chdir(dir) == -1) {
            perror("cd failed");
        }
    }
    char cwdTemp[PATH_MAX];
    getcwd(cwdTemp, PATH_MAX);
    currentDirectory = cwdTemp;
}
