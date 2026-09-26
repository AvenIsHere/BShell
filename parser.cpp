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

#include "parser.h"

#include <cctype>

Parser::Parser(const std::string& input) {
    this->input = input;
    this->state = NORMAL;
}

bool Parser::normal_token(const char token) {
    if (token == '\\') {
        state = ESCAPE;
        stack.push(NORMAL);
        return false;
    }
    if (token == '"') {
        state = QUOTE;
        stack.push(NORMAL);
        return false;
    }
    if (token == '$') {
        state = ENV_VAR;
        stack.push(NORMAL);
        return false;
    }
    if (token == ' ') {
        if (!current_token.empty()) {
            current_cmd.push_back(current_token);
            current_token.clear();
        }
        return false;
    }
    if (token == '\'') {
        state = SINGLE_QUOTE;
        stack.push(NORMAL);
        return false;
    }
    if (token == ';' || token == '\n') {
        if (!current_token.empty()) {
            current_cmd.push_back(current_token);
            current_token.clear();
        }
        if (!current_cmd.empty()) {
            return true;
        }
        return false;
    }
    current_token += token;
    return false;
}

bool Parser::quote_token(const char token) {
    if (token == '"') {
        state = stack.top();
        stack.pop();
        return false;
    }
    if (token == '\\') {
        state = ESCAPE;
        stack.push(QUOTE);
        return false;
    }
    if (token == '$') {
        state = ENV_VAR;
        stack.push(QUOTE);
        return false;
    }
    current_token += token;
    return false;
}

bool Parser::escape_token(const char token) {
    if (stack.top() != QUOTE) {
        if (token != '\n') {
            current_token += token;
        }
        state = stack.top();
        stack.pop();
        return false;
    }
    if (token == '$' || token == '`' || token == '"' || token == '\\') {
        current_token += token;
        state = stack.top();
        stack.pop();
        return false;
    }
    current_token += '\\';
    current_token += token;
    state = stack.top();
    stack.pop();
    return false;
}

bool Parser::env_var_token(const char token) {
    if (std::isalnum(token) || token == '_') {
        env_str += token;
        return false;
    }
    if (env_str.empty()) {
        current_token.push_back('$');
    }
    else if (const char* env = getenv(env_str.c_str())) {
        current_token.append(env);
    }
    env_str.clear();
    state = stack.top();
    stack.pop();
    return true;
}

bool Parser::single_quote_token(const char token) {
    if (token == '\'') {
        state = stack.top();
        stack.pop();
        return false;
    }
    current_token += token;
    return false;
}

std::vector<std::vector<std::string>> Parser::tokenise() {
    std::vector<std::vector<std::string>> return_vector;
    current_token.clear();
    current_cmd.clear();
    env_str.clear();
    for (int i = 0; i < input.length(); i++) {
        const char& c = input[i];
        switch (state) {
            case NORMAL: {
                if (normal_token(c)) {
                    return_vector.push_back(current_cmd);
                    current_cmd.clear();
                }
                continue;
            }
            case QUOTE: {
                if (quote_token(c)) {
                    return_vector.push_back(current_cmd);
                    current_cmd.clear();
                }
                continue;
            }
            case ESCAPE: {
                if (escape_token(c)) {
                    return_vector.push_back(current_cmd);
                    current_cmd.clear();
                }
                continue;
            }
            case ENV_VAR: {
                if (env_var_token(c)) {
                    i--;
                }
                continue;
            }
            case SINGLE_QUOTE: {
                if (single_quote_token(c)) {
                    return_vector.push_back(current_cmd);
                    current_cmd.clear();
                }
            }
        }
    }
    if (state == ENV_VAR) {
        if (env_str.empty()) {
            current_token.push_back('$');
        }
        else if (const char* env = getenv(env_str.c_str())) {
            current_token.append(env);
        }
    }
    if (!current_token.empty()) {
        current_cmd.push_back(current_token);
    }
    if (!current_cmd.empty()) {
        return_vector.push_back(current_cmd);
    }
    while (!stack.empty()) {
        stack.pop();
    }
    return return_vector;
}