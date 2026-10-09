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

#ifndef BSHELL_CONFIG_H
#define BSHELL_CONFIG_H
#include <string>
#include <unordered_set>
#include <vector>


class Config {
    static std::string home_path;
    static std::string current_directory;
    static std::string username;
    static std::string hostname;
    static std::string pipe_delim;
    static std::string path_str;
    static std::unordered_set<std::string> commands;

public:
    static void set_current_directory(const std::string &given_current_directory) {
        current_directory = given_current_directory;
    }

    static std::string get_home_path() {
        return home_path;
    }

    static std::string get_current_directory() {
        return current_directory;
    }

    static std::string get_username() {
        return username;
    }

    static std::string get_hostname() {
        return hostname;
    }

    static std::string get_pipe_delim() {
        return pipe_delim;
    }

    static std::unordered_set<std::string> get_commands() {
        return commands;
    }

    static void init();

    static void build_commands();
    static void export_env(const std::vector<std::string> &given_command);
    static void cd(const std::vector<std::string> &given_command);
};


#endif //BSHELL_CONFIG_H
