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

void Parser::normal_token(const char token) {
    if (token == '\\') {
        state = ESCAPE;
        stack.push(NORMAL);
        return;
    }
    if (token == '"') {
        state = QUOTE;
        stack.push(NORMAL);
        return;
    }
    if (token == '$') {
        state = ENV_VAR;
        stack.push(NORMAL);
        return;
    }
    if (token == ' ') {
        if (!current_token.empty()) {
            current_cmd.push_back(current_token);
            current_token.clear();
        }
        return;
    }
    if (token == '\'') {
        state = SINGLE_QUOTE;
        stack.push(NORMAL);
        return;
    }
    if (token == ';' || token == '\n') {
        if (!current_token.empty()) {
            current_cmd.push_back(current_token);
            current_token.clear();
        }
        if (!current_cmd.empty()) {
            parsed_input.push_back(current_cmd);
            current_cmd.clear();
        }
        return;
    }
    current_token += token;
}

void Parser::quote_token(const char token) {
    if (token == '"') {
        state = stack.top();
        stack.pop();
        return;
    }
    if (token == '\\') {
        state = ESCAPE;
        stack.push(QUOTE);
        return;
    }
    if (token == '$') {
        state = ENV_VAR;
        stack.push(QUOTE);
        return;
    }
    current_token += token;
}

void Parser::escape_token(const char token) {
    if (stack.top() != QUOTE) {
        if (token != '\n') {
            current_token += token;
        }
        state = stack.top();
        stack.pop();
        return;
    }
    if (token == '$' || token == '`' || token == '"' || token == '\\') {
        current_token += token;
        state = stack.top();
        stack.pop();
        return;
    }
    current_token += '\\';
    current_token += token;
    state = stack.top();
    stack.pop();
}

void Parser::env_var_token(const char token) {
    if (std::isalnum(token) || token == '_') {
        env_str += token;
        return;
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
    index--;
}

void Parser::single_quote_token(const char token) {
    if (token == '\'') {
        state = stack.top();
        stack.pop();
        return;
    }
    current_token += token;
}

std::vector<Command> Parser::tokenise() {
    parsed_input.clear();
    current_token.clear();
    current_cmd.clear();
    env_str.clear();
    for (; index < input.length(); index++) {
        const char& c = input[index];
        switch (state) {
            case NORMAL: {
                normal_token(c);
                continue;
            }
            case QUOTE: {
                quote_token(c);
                continue;
            }
            case ESCAPE: {
                escape_token(c);
                continue;
            }
            case ENV_VAR: {
                env_var_token(c);
                continue;
            }
            case SINGLE_QUOTE: {
                single_quote_token(c);
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
        parsed_input.push_back(current_cmd);
    }
    stack = {};
    return parsed_input;
}