#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

struct JSONObject;
using JSONList = std::vector<JSONObject>;
using JSONDict = std::unordered_map<std::string, JSONObject>;

struct JSONObject {
    std::variant<
        std::monostate,  // null
        bool,            // true or false
        int,             // integer
        double,          // floating point
        std::string,     // string
        JSONList,        // list
        JSONDict         // dict
        >
        value;

    JSONObject() : value(std::monostate{}) {}
    JSONObject(std::monostate) : value(std::monostate{}) {}
    JSONObject(bool value) : value(value) {}
    JSONObject(int value) : value(value) {}
    JSONObject(double value) : value(value) {}
    JSONObject(std::string value) : value(value) {}
    JSONObject(std::vector<JSONObject> value) : value(value) {}
    JSONObject(std::unordered_map<std::string, JSONObject> value) : value(value) {}

    std::string to_string() const {
        if (value.index() == 0) {
            return "null";
        } else if (value.index() == 1) {
            return std::get<bool>(value) ? "true" : "false";
        } else if (value.index() == 2) {
            return std::to_string(std::get<int>(value));
        } else if (value.index() == 3) {
            return std::to_string(std::get<double>(value));
        } else if (value.index() == 4) {
            return "\"" + std::get<std::string>(value) + "\"";
        } else if (value.index() == 5) {
            std::string result = "[";
            for (const JSONObject& obj : std::get<std::vector<JSONObject>>(value)) {
                result += obj.to_string() + ", ";
            }
            if (result.size() > 1) {
                result.pop_back();
                result.pop_back();
            }
            result += "]";
            return result;
        } else if (value.index() == 6) {
            std::string result = "{";
            for (const auto& [key, obj] : std::get<std::unordered_map<std::string, JSONObject>>(value)) {
                result += "\"" + key + "\": " + obj.to_string() + ", ";
            }
            if (result.size() > 1) {
                result.pop_back();
                result.pop_back();
            }
            result += "}";
            return result;
        }
        return "";
    }

    template <class T>
    bool is() const {
        return std::holds_alternative<T>(value);
    }

    template <class T>
    T const& get() const {
        return std::get<T>(value);
    }

    template <class T>
    T& get() {
        return std::get<T>(value);
    }
};

char unescaped_char(char ch) {
    switch (ch) {
        case 'r':
            return '\r';
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case '0':
            return '\0';
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        default:
            return ch;
    }
}
