#include <charconv>
#include <iostream>
#include <optional>
#include <regex>

#include "json_object.hpp"

void interactive(JSONObject& obj) {
    std::string command;
    while (true) {
        std::cout << "Enter a command (obj, obj[0], obj[\"key\"], exit): ";
        std::getline(std::cin, command);
        if (command == "exit") {
            break;
        } else if (command == "obj") {
            std::cout << obj.to_string() << std::endl;
        } else if (command.substr(0, 4) == "obj[") {
            int index = std::stoi(command.substr(4, command.length() - 5));
            std::cout << obj[index].to_string() << std::endl;
        } else if (command.substr(0, 5) == "obj[\"") {
            std::string key = command.substr(5, command.length() - 6);
            std::cout << obj[key].to_string() << std::endl;
        } else {
            std::cout << "Invalid command." << std::endl;
        }
    }
}

template <class T>
std::optional<T> parse_num(std::string_view str) {
    T value = 0;
    auto res = std::from_chars(str.data(), str.data() + str.size(), value);
    if (res.ec == std::errc::invalid_argument ||
        res.ec == std::errc::result_out_of_range ||
        res.ptr != str.data() + str.size()) {
        return std::nullopt;
    }
    return value;
}

std::pair<JSONObject, size_t> parse(const std::string_view json) {
    if (json.empty()) {
        // null
        return std::make_pair(JSONObject(std::monostate{}), 0);
    }
    if (json.front() == '-' || json.front() == '+' || std::isdigit(json.front())) {
        // if it is a integer, or a floating point number
        std::regex num_regex("[+-]?\\d+(\\.\\d+)?([eE][+-]?\\d+)?");
        std::cmatch match;
        if (std::regex_match(json.begin(), json.end(), match, num_regex)) {
            std::string num_str = match.str();
            if (auto num = parse_num<int>(num_str)) {
                return std::make_pair(JSONObject(*num), num_str.size());
            } else if (auto num = parse_num<double>(num_str)) {
                return std::make_pair(JSONObject(*num), num_str.size());
            }
        }
        throw std::runtime_error("Invalid number: " + std::string(json));
        return std::make_pair(JSONObject(), -1);
    } else if (json[0] == '"') {
        // if it is a string
        std::string str;
        enum {
            NORMAL,
            ESCAPE,
        } state = NORMAL;
        for (size_t i = 1; i < json.size(); i++) {
            char ch = json[i];
            if (state == NORMAL) {
                if (ch == '\\') {
                    state = ESCAPE;
                } else if (ch == '"') {
                    break;
                } else {
                    str += ch;
                }
            } else if (state == ESCAPE) {
                str += unescaped_char(ch);
                state = NORMAL;
            }
        }
        return std::make_pair(JSONObject(move(str)), str.size() + 2);
    } else if (json[0] == '[') {
        // if it is a list
        std::vector<JSONObject> list;
    }

    return std::make_pair(JSONObject(), -1);
}

int main() {
    // std::string str = "-3.14e-2";
    std::string str = R"JSON([1,2,3])JSON";
    std::cout << str << std::endl;
    auto [obj, len] = parse(str);
    // interactive(obj);
    return 0;
}