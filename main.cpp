#include <charconv>
#include <iostream>
#include <optional>
#include <regex>

#include "json_object.hpp"

const std::string NUM_REG = "[+-]?\\d+(\\.\\d+)?([eE][+-]?\\d+)?";
const std::string WHITESPACES = " \t\n\r\f\v\0";

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
    } else if (size_t off = json.find_first_not_of(WHITESPACES); off != 0 and off != json.npos) {
        // remove whitespace
        auto [obj, len] = parse(json.substr(off));
        return std::make_pair(std::move(obj), off + len);
    } else if (json.front() == '-' || json.front() == '+' || std::isdigit(json.front())) {
        // if it is a integer, or a floating point number
        std::regex num_reg(NUM_REG);
        std::cmatch match;
        if (std::regex_search(json.data(), json.data() + json.size(), match, num_reg)) {
            std::string num_str = match.str();
            if (auto num = parse_num<int>(num_str)) {
                return std::make_pair(JSONObject(*num), num_str.size());
            } else if (auto num = parse_num<double>(num_str)) {
                return std::make_pair(JSONObject(*num), num_str.size());
            }
        }
        return std::make_pair(JSONObject(), 0);
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
        // str.size() + 2 is the length of the string + 2 double quotes
        return std::make_pair(JSONObject(move(str)), str.size() + 2);
    } else if (json[0] == '[') {
        // if it is a list
        std::vector<JSONObject> list;
        size_t i = 1;
        for (; i < json.size();) {
            if (json[i] == ']') {
                i += 1;
                break;
            } else {
                auto [obj, len] = parse(json.substr(i));
                if (len == 0) {
                    i = 0;
                    break;
                }
                list.push_back(std::move(obj));
                i += len;
                if (json[i] == ',') {
                    i += 1;
                }
            }
        }
        // i + 1 is the length of the list
        return std::make_pair(JSONObject(move(list)), i);
    } else if (json[0] == '{') {
        // if it is a dict
        std::unordered_map<std::string, JSONObject> dict;
        size_t i = 1;
        for (; i < json.size();) {
            if (json[i] == '}') {
                i += 1;
                break;
            }
            auto [key, keylen] = parse(json.substr(i));
            if (keylen == 0) {
                // invalid key
                i = 0;
                break;
            }
            i += keylen;
            if (json[i] == '}') {
                // no more key-value pair
                i += 1;
                break;
            }
            if (!std::holds_alternative<std::string>(key.value)) {
                // key is not a string
                i = 0;
                break;
            }
            if (json[i] == ':') {
                i += 1;
            }
            std::string key_str = std::move(std::get<std::string>(key.value));
            auto [val, vallen] = parse(json.substr(i));
            if (vallen == 0) {
                // invalid value
                i = 0;
                break;
            }
            i += vallen;
            auto [is_dup, it] = dict.insert_or_assign(std::move(key_str), std::move(val));
            // we could handle duplicate keys here, but we just choose to update the value
            if (json[i] == ',') {
                i += 1;
            }
        }
        // i + 1 is the length of the dict
        return std::make_pair(JSONObject(move(dict)), i);
    }

    return std::make_pair(JSONObject(), 0);
}

int main() {
    // std::string str = "-3.14e-2";
    // std::string str = R"({"work":996,"school":[985,211],"my_school":{"name":"UofT","rank":21}})";
    std::string str = R"({
        "work": 996,
        "school": [985, 211],
        "my_school":{
            "name": "UofT",
            "rank": 21
            }
        }
    )";
    auto [obj, len] = parse(str);
    std::cout << obj.to_string() << std::endl;
    std::cout << len << std::endl;
    return 0;
}