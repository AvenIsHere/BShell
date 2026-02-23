//
// Created by aven on 23/02/2026.
//

#ifndef BASICSHELL_CONFIG_H
#define BASICSHELL_CONFIG_H
#include <string>
#include <vector>


class Config {
public:
    void set_current_directory(const std::string &current_directory) {
        currentDirectory = current_directory;
    }

    [[nodiscard]] const char *get_home_path() const {
        return homePath;
    }

    [[nodiscard]] std::string get_current_directory() const {
        return currentDirectory;
    }

    [[nodiscard]] std::string get_username() const {
        return username;
    }

    [[nodiscard]] std::string get_hostname() const {
        return hostname;
    }

    [[nodiscard]] std::string get_pipe_delim() const {
        return pipeDelim;
    }

private:
    const char *homePath;
    std::string currentDirectory;
    std::string username;
    std::string hostname;
    std::string pipeDelim;

public:
    Config();

    void cd(const std::vector<std::string> &givenCommand);
};


#endif //BASICSHELL_CONFIG_H
