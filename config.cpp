// BShell - A shell for POSIX-compliant operating systems
// Copyright (C) 2026 Aven Furness
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#endif

std::string Config::home_path;
std::string Config::username;
std::string Config::current_directory;
std::string Config::hostname;
std::string Config::pipe_delim;
std::string Config::path_str;
std::unordered_set<std::string> Config::commands;

void Config::init() {
    home_path = getenv("HOME");

    char current_directory_temp[PATH_MAX];
    if (getcwd(current_directory_temp, PATH_MAX) == nullptr) {
        perror("Could not find current directory");
        exit(1);
    }
    current_directory = current_directory_temp;

    const char* username_temp = getenv("USER");
    username = username_temp == nullptr ? "unknown" : username_temp;

    const char* pipe_delim_temp = getenv("PIPE_DELIM");
    pipe_delim = pipe_delim_temp == nullptr ? "|" : pipe_delim_temp;

    signal(SIGINT, SIG_IGN);

    char hostname_temp[HOST_NAME_MAX];
    gethostname(hostname_temp, HOST_NAME_MAX);
    hostname = hostname_temp;

    build_commands();
}

void Config::cd(const std::vector<std::string> &given_command) {
    if (given_command.size() < 2) {
        if (!home_path.empty()) {
            chdir(home_path.c_str());
        }
    } else if (given_command.size() > 2) {
        std::cerr << "Too many arguments." << std::endl;
    } else {
        errno = 0;
        std::string dir;
        const std::string& path = given_command[1];
        if (path[0] == '~' && !home_path.empty()) {
            dir = std::format("{}{}", home_path, path.substr(1));
        } else {
            dir = path;
        }
        if (chdir(dir.c_str()) == -1) {
            perror("cd failed");
        }
    }
    char cwd_temp[PATH_MAX];
    getcwd(cwd_temp, PATH_MAX);
    current_directory = cwd_temp;
}

void Config::export_env(const std::vector<std::string> &given_command) {
    if (given_command.size() < 2) {
        for (char** env = environ; *env != nullptr; env++) {
            std::cout << *env << std::endl;
        }
    }
    for (int i=1; i < given_command.size(); i++) {
        const std::string& entry = given_command[i];
        if (const size_t pos = entry.find('='); pos != std::string::npos) {
            std::string key = entry.substr(0, pos);
            std::string value = entry.substr(pos + 1);
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
}

void Config::build_commands() {
    commands.clear();
    const char* path_env = getenv("PATH");
    if (!path_env) return;

    path_str = path_env;
    std::stringstream ss(path_env);
    std::string dir_path;
    while (std::getline(ss, dir_path, ':')) {
        try {
            if (!std::filesystem::exists(dir_path) || !std::filesystem::is_directory(dir_path)) continue;
            for (const auto& entry : std::filesystem::directory_iterator(dir_path)) {
                if (!entry.is_regular_file()) {
                    continue;
                }
                if (auto status = entry.status(); (status.permissions() & std::filesystem::perms::owner_exec) != std::filesystem::perms::none) {
                    commands.insert(entry.path().filename().string());
                }
            }
        } catch (const std::filesystem::filesystem_error& error) {
            std::cerr << "Error reading PATH files: " << error.what() << std::endl;
        }
    }
}