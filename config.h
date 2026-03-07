//
// Created by aven on 23/02/2026.
//

#ifndef BSHELL_CONFIG_H
#define BSHELL_CONFIG_H
#include <string>
#include <unordered_set>
#include <vector>


class Config {
private:
    std::string home_path;
    std::string current_directory;
    std::string username;
    std::string hostname;
    std::string pipe_delim;
    std::unordered_set<std::string> commands;

public:
    void set_current_directory(const std::string &given_current_directory) {
        this->current_directory = given_current_directory;
    }

    [[nodiscard]] std::string get_home_path() const {
        return home_path;
    }

    [[nodiscard]] std::string get_current_directory() const {
        return current_directory;
    }

    [[nodiscard]] std::string get_username() const {
        return username;
    }

    [[nodiscard]] std::string get_hostname() const {
        return hostname;
    }

    [[nodiscard]] std::string get_pipe_delim() const {
        return pipe_delim;
    }

    [[nodiscard]] std::unordered_set<std::string> get_commands() const {
        return commands;
    }

    Config();

    void build_commands();
    void cd(const std::vector<std::string> &givenCommand);
};


#endif //BSHELL_CONFIG_H
