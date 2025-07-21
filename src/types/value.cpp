#include "value.h"
#include <iomanip> // For std::fixed, std::setprecision

// Implementation of MegaladonValue::toString()
std::string MegaladonValue::toString() const {
    switch (type) {
        case VOID: return "void";
        case NUMBER: {
            double num = std::get<double>(data);
            if (std::fmod(num, 1.0) == 0.0) { // Check if it's an integer
                return std::to_string(static_cast<long long>(num));
            } else {
                // Use std::fixed and std::setprecision for consistent double output
                std::stringstream ss;
                ss << std::fixed << std::setprecision(6) << num; // Adjust precision as needed
                std::string s = ss.str();
                // Remove trailing zeros and decimal point if it's a whole number
                s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                if (s.back() == '.') {
                    s.pop_back();
                }
                return s;
            }
        }
        case BOOLEAN: return std::get<bool>(data) ? "true" : "false";
        case STRING: return std::get<std::string>(data);
        case LIST: {
            std::string s = "[";
            const auto& list = std::get<std::vector<MegaladonValue>>(data);
            for (size_t i = 0; i < list.size(); ++i) {
                s += list[i].toString();
                if (i < list.size() - 1) {
                    s += ", ";
                }
            }
            s += "]";
            return s;
        }
        case FUNCTION: return std::get<std::shared_ptr<MegaladonCallable>>(data)->toString();
        case INVALID: return "invalid";
        default: return "unknown";
    }
}

// Implementation of equality operator
bool operator==(const MegaladonValue& lhs, const MegaladonValue& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }

    switch (lhs.type) {
        case VOID:
        case INVALID:
            return true; // VOID == VOID, INVALID == INVALID
        case NUMBER:
            return lhs.asNumber() == rhs.asNumber();
        case BOOLEAN:
            return lhs.asBoolean() == rhs.asBoolean();
        case STRING:
            return lhs.asString() == rhs.asString();
        case LIST:
            return lhs.asList() == rhs.asList(); // std::vector has operator==
        case FUNCTION:
            // Compare shared_ptr raw pointers or a custom ID for functions
            return lhs.asCallable() == rhs.asCallable();
        default:
            return false; // Should not reach here for defined types
    }
}

// Implementation of inequality operator
bool operator!=(const MegaladonValue& lhs, const MegaladonValue& rhs) {
    return !(lhs == rhs);
}