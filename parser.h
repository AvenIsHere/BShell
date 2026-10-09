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

#ifndef BSHELL_PARSER_H
#define BSHELL_PARSER_H
#include <stack>
#include <string>
#include <vector>

using Command = std::vector<std::string>;


class Parser {
    enum State {
        NORMAL,
        QUOTE,
        ESCAPE,
        ENV_VAR,
        SINGLE_QUOTE,
    };

    std::string input;

    State state;
    std::stack<State> stack;
    size_t index = 0;

    std::vector<Command> parsed_input;
    std::string current_token;
    Command current_cmd;
    std::string env_str;

public:
    explicit Parser(const std::string& input);
    std::vector<Command> tokenise();

    void normal_token(char token);
    void quote_token(char token);
    void escape_token(char token);
    void env_var_token(char token);
    void single_quote_token(char token);
};


#endif //BSHELL_PARSER_H