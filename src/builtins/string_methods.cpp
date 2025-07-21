// src/builtins/string_methods.cpp
#include "../types/value.h"
#include "../util/error.h"
#include <algorithm> // For std::transform, std::find_if
#include <cctype>    // For ::tolower, ::toupper, ::isspace

// Each function below implements a string method.
// The first argument in `args` is always the `this` object (the string itself).

// String Methods
// 1. String.len()
// 2. String.substring(start, end)
// 3. String.to_lower()
// 4. String.to_upper()
// 5. String.trim()
// 6. String.starts_with(prefix)
// 7. String.ends_with(suffix)
// 8. String.contains(substring)
// 9. String.replace(old, new)
// 10. String.split(delimiter)
// 11. String.index_of(substring, start_index)
// 12. String.to_list()
// 13. String.count_vowels()

MegaladonValue string_len(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string]
    if (args.size() != 1 || !args[0].isString()) {
        ReportRuntimeError("String.len() expects one string argument.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    return MegaladonValue(static_cast<double>(args[0].asString().length()));
}

MegaladonValue string_substring(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string, startIndex, [endIndex]]
    if (args.size() < 2 || args.size() > 3 || !args[0].isString() || !args[1].isNumber()) {
        ReportRuntimeError("String.substring(startIndex, [endIndex]) expects string, start_index (number), and optional end_index (number).");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::string s = args[0].asString();
    size_t start = static_cast<size_t>(args[1].asNumber());
    size_t end = s.length(); // Default to end of string

    if (args.size() == 3) {
        if (!args[2].isNumber()) {
            ReportRuntimeError("String.substring() end_index must be a number.");
            return MegaladonValue(MegaladonValue::INVALID);
        }
        end = static_cast<size_t>(args[2].asNumber());
    }

    if (start >= s.length()) return MegaladonValue("");
    if (end > s.length()) end = s.length();
    if (start > end) return MegaladonValue("");

    return MegaladonValue(s.substr(start, end - start));
}

MegaladonValue string_to_lower(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string]
    if (args.size() != 1 || !args[0].isString()) {
        ReportRuntimeError("String.to_lower() expects one string argument.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::string s = args[0].asString();
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return MegaladonValue(s);
}

MegaladonValue string_to_upper(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string]
    if (args.size() != 1 || !args[0].isString()) {
        ReportRuntimeError("String.to_upper() expects one string argument.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::string s = args[0].asString();
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return MegaladonValue(s);
}

MegaladonValue string_trim(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string]
    if (args.size() != 1 || !args[0].isString()) {
        ReportRuntimeError("String.trim() expects one string argument.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::string s = args[0].asString();
    // Trim leading whitespace
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    // Trim trailing whitespace
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return MegaladonValue(s);
}

MegaladonValue string_starts_with(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string, prefix_string]
    if (args.size() != 2 || !args[0].isString() || !args[1].isString()) {
        ReportRuntimeError("String.starts_with(prefix) expects two string arguments.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const std::string& str = args[0].asString();
    const std::string& prefix = args[1].asString();
    return MegaladonValue(str.rfind(prefix, 0) == 0);
}

MegaladonValue string_ends_with(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string, suffix_string]
    if (args.size() != 2 || !args[0].isString() || !args[1].isString()) {
        ReportRuntimeError("String.ends_with(suffix) expects two string arguments.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const std::string& str = args[0].asString();
    const std::string& suffix = args[1].asString();
    if (str.length() < suffix.length()) return MegaladonValue(false);
    return MegaladonValue(str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0);
}

MegaladonValue string_contains(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string, substring]
    if (args.size() != 2 || !args[0].isString() || !args[1].isString()) {
        ReportRuntimeError("String.contains(substring) expects two string arguments.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const std::string& str = args[0].asString();
    const std::string& substr = args[1].asString();
    return MegaladonValue(str.find(substr) != std::string::npos);
}

MegaladonValue string_replace(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string, old_substring, new_substring]
    if (args.size() != 3 || !args[0].isString() || !args[1].isString() || !args[2].isString()) {
        ReportRuntimeError("String.replace(old, new) expects three string arguments: original, old_substring, new_substring.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::string str = args[0].asString();
    const std::string& old_substr = args[1].asString();
    const std::string& new_substr = args[2].asString();

    size_t pos = 0;
    while ((pos = str.find(old_substr, pos)) != std::string::npos) {
        str.replace(pos, old_substr.length(), new_substr);
        pos += new_substr.length();
    }
    return MegaladonValue(str);
}

MegaladonValue string_split(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string, [delimiter_string]]
    if (args.size() < 1 || args.size() > 2 || !args[0].isString()) {
        ReportRuntimeError("String.split([delimiter]) expects string and optional delimiter (string).");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::string s = args[0].asString();
    std::string delimiter = (args.size() == 2 && args[1].isString()) ? args[1].asString() : " "; // Default delimiter is space

    std::vector<MegaladonValue> result_list;
    size_t pos = 0;
    std::string token;
    // Handle empty delimiter edge case (splits into chars)
    if (delimiter.empty()) {
        for(char c : s) {
            result_list.push_back(MegaladonValue(std::string(1, c)));
        }
        return MegaladonValue(result_list);
    }

    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        result_list.push_back(MegaladonValue(token));
        s.erase(0, pos + delimiter.length());
    }
    result_list.push_back(MegaladonValue(s)); // Add the last part

    return MegaladonValue(result_list);
}

MegaladonValue string_index_of(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string, substring, [startIndex]]
    if (args.size() < 2 || args.size() > 3 || !args[0].isString() || !args[1].isString()) {
        ReportRuntimeError("String.index_of(substring, [startIndex]) expects string, substring (string), and optional start_index (number).");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const std::string& str = args[0].asString();
    const std::string& substr = args[1].asString();
    size_t start_pos = 0;
    if (args.size() == 3) {
        if (!args[2].isNumber()) {
            ReportRuntimeError("String.index_of() start_index must be a number.");
            return MegaladonValue(MegaladonValue::INVALID);
        }
        start_pos = static_cast<size_t>(args[2].asNumber());
    }

    size_t found_pos = str.find(substr, start_pos);
    if (found_pos != std::string::npos) {
        return MegaladonValue(static_cast<double>(found_pos));
    } else {
        return MegaladonValue(-1.0); // -1 if not found
    }
}

MegaladonValue string_to_list(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string]
    if (args.size() != 1 || !args[0].isString()) {
        ReportRuntimeError("String.to_list() expects one string argument.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::string s = args[0].asString();
    std::vector<MegaladonValue> char_list;
    for (char c : s) {
        char_list.push_back(MegaladonValue(std::string(1, c)));
    }
    return MegaladonValue(char_list);
}

MegaladonValue string_count_vowels(const std::vector<MegaladonValue>& args) {
    // Expects: [this_string]
    if (args.size() != 1 || !args[0].isString()) {
        ReportRuntimeError("String.count_vowels() expects one string argument.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::string s = args[0].asString();
    int vowels = 0;
    for (char c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); // Correct usage of tolower
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
            vowels++;
        }
    }
    return MegaladonValue(static_cast<double>(vowels));
}