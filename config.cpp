//
// Created by aven on 23/02/2026.
//

#include "config.h"

#include <csignal>
#include <unistd.h>
#include <vector>
#include <climits>
#include <filesystem>
#include <format>
#include <iostream>
#include <sstream>

#ifndef HOST_NAME_MAX
  #if defined(_POSIX_HOST_NAME_MAX)
    #define HOST_NAME_MAX _POSIX_HOST_NAME_MAX
  #else
    #define HOST_NAME_MAX 255
  #endif
#endif

Config::Config() {
    const char* homePathTemp = getenv("HOME");
    this->home_path = homePathTemp;

    char currentDirectoryTemp[PATH_MAX];
    if (getcwd(currentDirectoryTemp, PATH_MAX) == nullptr) {
        perror("Could not find current directory");
        exit(1);
    }
    this->current_directory = currentDirectoryTemp;

    const char* usernameTemp = getenv("USER");
    std::string usernameStr;
    if (usernameTemp == nullptr) {
        usernameStr = "unknown";
    } else {
        usernameStr = usernameTemp;
    }
    this->username = usernameStr;

    const char* pipeDelimTemp = getenv("PIPE_DELIM");
    std::string pipeDelim;
    if (pipeDelimTemp == nullptr) {
        pipeDelim = "|";
    } else {
        pipeDelim = pipeDelimTemp;
    }
    this->pipe_delim = pipeDelim;

    signal(SIGINT, SIG_IGN);

    char hostnameTemp[HOST_NAME_MAX];
    gethostname(hostnameTemp, HOST_NAME_MAX);
    const std::string hostnameStr = hostnameTemp;
    this->hostname = hostnameStr;

    build_commands();
}

void Config::cd(const std::vector<std::string> &givenCommand) {
    if (givenCommand.size() < 2) {
        if (home_path.c_str() != nullptr) {
            chdir(home_path.c_str());
        }
    } else if (givenCommand.size() > 2) {
        std::cerr << "Too many arguments." << std::endl;
    } else {
        errno = 0;
        std::string dir;
        if (givenCommand[1][0] == '~' && home_path.c_str() != nullptr) {
            dir = std::format("{}{}", home_path, givenCommand[1].c_str() + 1);
        } else {
            dir = givenCommand[1];
        }
        if (chdir(dir.c_str()) == -1) {
            perror("cd failed");
        }
    }
    char cwdTemp[PATH_MAX];
    getcwd(cwdTemp, PATH_MAX);
    current_directory = cwdTemp;

    build_commands();
}

void Config::build_commands() {
    commands.clear();
    const char* path_env = getenv("PATH");
    if (!path_env) return;

    std::stringstream ss(path_env);
    std::string dir_path;
    while (std::getline(ss, dir_path, ':')) {
        try {
            if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) continue;
            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                if (entry.is_regular_file()) {
                    if (auto status = entry.status(); (status.permissions() & std::filesystem::perms::owner_exec) != std::filesystem::perms::none) {
                        commands.insert(entry.path().filename().string());
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error&) {}
    }
}