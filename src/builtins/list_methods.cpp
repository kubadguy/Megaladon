// src/builtins/list_methods.cpp
#include "../types/value.h"
#include "../util/error.h"
#include "../interpreter/interpreter.h" // For Interpreter when calling higher-order functions
#include <algorithm> // For std::reverse, std::sort, std::remove_if

// List Methods
// 1. List.len()
// 2. List.add(item)
// 3. List.remove_at(index)
// 4. List.get(index) (already handled by direct indexing, but good to have explicit method)
// 5. List.set(index, value) (already handled by direct indexing assignment)
// 6. List.contains(item)
// 7. List.insert_at(index, item)
// 8. List.pop([index])
// 9. List.clear()
// 10. List.join(delimiter)
// 11. List.reverse()
// 12. List.slice(start, end)
// 13. List.remove(item)
// 14. List.sort()
// 15. List.filter(func)
// 16. List.map(func)

MegaladonValue list_len(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list]
    if (args.size() != 1 || !args[0].isList()) {
        ReportRuntimeError("List.len() expects one list argument.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    return MegaladonValue(static_cast<double>(args[0].asList().size()));
}

MegaladonValue list_add(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, item]
    if (args.size() != 2 || !args[0].isList()) {
        ReportRuntimeError("List.add(item) expects a list and an item.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    args[0].asListMutable().push_back(args[1]);
    return MegaladonValue(); // Return void
}

MegaladonValue list_remove_at(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, index_number]
    if (args.size() != 2 || !args[0].isList() || !args[1].isNumber()) {
        ReportRuntimeError("List.remove_at(index) expects a list and an index (number).");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    auto& list = args[0].asListMutable();
    int index = static_cast<int>(args[1].asNumber());
    if (index < 0 || index >= list.size()) {
        ReportRuntimeError("List index out of bounds: " + std::to_string(index));
        return MegaladonValue(MegaladonValue::INVALID);
    }
    MegaladonValue removed_item = list[index];
    list.erase(list.begin() + index);
    return removed_item; // Return the removed item
}

MegaladonValue list_get(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, index_number]
    if (args.size() != 2 || !args[0].isList() || !args[1].isNumber()) {
        ReportRuntimeError("List.get(index) expects a list and an index (number).");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const auto& list = args[0].asList();
    int index = static_cast<int>(args[1].asNumber());
    if (index < 0 || index >= list.size()) {
        ReportRuntimeError("List index out of bounds: " + std::to_string(index));
        return MegaladonValue(MegaladonValue::INVALID);
    }
    return list[index];
}

MegaladonValue list_set(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, index_number, value]
    if (args.size() != 3 || !args[0].isList() || !args[1].isNumber()) {
        ReportRuntimeError("List.set(index, value) expects a list, an index (number), and a value.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    auto& list = args[0].asListMutable();
    int index = static_cast<int>(args[1].asNumber());
    if (index < 0 || index >= list.size()) {
        ReportRuntimeError("List index out of bounds: " + std::to_string(index));
        return MegaladonValue(MegaladonValue::INVALID);
    }
    list[index] = args[2];
    return MegaladonValue(); // Return void
}

MegaladonValue list_contains(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, item]
    if (args.size() != 2 || !args[0].isList()) {
        ReportRuntimeError("List.contains(item) expects a list and an item.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const auto& list = args[0].asList();
    const MegaladonValue& item = args[1];
    for (const auto& val : list) {
        if (val == item) return MegaladonValue(true);
    }
    return MegaladonValue(false);
}

MegaladonValue list_insert_at(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, index_number, item]
    if (args.size() != 3 || !args[0].isList() || !args[1].isNumber()) {
        ReportRuntimeError("List.insert_at(index, item) expects a list, an index (number), and a value.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    auto& list = args[0].asListMutable();
    int index = static_cast<int>(args[1].asNumber());
    if (index < 0 || index > list.size()) { // Allow inserting at list.size() for append
        ReportRuntimeError("List index out of bounds for insert: " + std::to_string(index));
        return MegaladonValue(MegaladonValue::INVALID);
    }
    list.insert(list.begin() + index, args[2]);
    return MegaladonValue(); // Return void
}

MegaladonValue list_pop(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, [index_number]]
    if (args.size() < 1 || args.size() > 2 || !args[0].isList()) {
        ReportRuntimeError("List.pop([index]) expects a list and an optional index (number).");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    auto& list = args[0].asListMutable();
    if (list.empty()) {
        ReportRuntimeError("Cannot pop from an empty list.");
        return MegaladonValue(MegaladonValue::INVALID);
    }

    int index_to_pop;
    if (args.size() == 1) { // Pop last element if no index given
        index_to_pop = static_cast<int>(list.size() - 1);
    } else { // Pop at specified index
        if (!args[1].isNumber()) {
            ReportRuntimeError("List.pop() index must be a number.");
            return MegaladonValue(MegaladonValue::INVALID);
        }
        index_to_pop = static_cast<int>(args[1].asNumber());
    }

    if (index_to_pop < 0 || index_to_pop >= list.size()) {
        ReportRuntimeError("List index out of bounds for pop: " + std::to_string(index_to_pop));
        return MegaladonValue(MegaladonValue::INVALID);
    }

    MegaladonValue removed_item = list[index_to_pop];
    list.erase(list.begin() + index_to_pop);
    return removed_item;
}

MegaladonValue list_clear(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list]
    if (args.size() != 1 || !args[0].isList()) {
        ReportRuntimeError("List.clear() expects a list.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    args[0].asListMutable().clear();
    return MegaladonValue(); // Return void
}

MegaladonValue list_join(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, [delimiter_string]]
    if (args.size() < 1 || args.size() > 2 || !args[0].isList()) {
        ReportRuntimeError("List.join([delimiter]) expects a list and optional delimiter (string).");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const auto& list = args[0].asList();
    std::string delimiter = (args.size() == 2 && args[1].isString()) ? args[1].asString() : ""; // Default delimiter is empty string

    std::string result_str = "";
    for (size_t i = 0; i < list.size(); ++i) {
        result_str += list[i].toString();
        if (i < list.size() - 1) {
            result_str += delimiter;
        }
    }
    return MegaladonValue(result_str);
}

MegaladonValue list_reverse(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list]
    if (args.size() != 1 || !args[0].isList()) {
        ReportRuntimeError("List.reverse() expects a list.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    std::vector<MegaladonValue> reversed_list = args[0].asList(); // Make a copy
    std::reverse(reversed_list.begin(), reversed_list.end());
    return MegaladonValue(reversed_list);
}

MegaladonValue list_slice(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, start_index_number, [end_index_number]]
    if (args.size() < 2 || args.size() > 3 || !args[0].isList() || !args[1].isNumber()) {
        ReportRuntimeError("List.slice(start, [end]) expects list, start_index (number), and optional end_index (number).");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const auto& original_list = args[0].asList();
    size_t start = static_cast<size_t>(args[1].asNumber());
    size_t end = original_list.size(); // Default to end of list

    if (args.size() == 3) {
        if (!args[2].isNumber()) {
            ReportRuntimeError("List.slice() end_index must be a number.");
            return MegaladonValue(MegaladonValue::INVALID);
        }
        end = static_cast<size_t>(args[2].asNumber());
    }

    if (start >= original_list.size()) return MegaladonValue(std::vector<MegaladonValue>{});
    if (end > original_list.size()) end = original_list.size();
    if (start > end) return MegaladonValue(std::vector<MegaladonValue>{});

    std::vector<MegaladonValue> sliced_list;
    for (size_t i = start; i < end; ++i) {
        sliced_list.push_back(original_list[i]);
    }
    return MegaladonValue(sliced_list);
}

MegaladonValue list_remove(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, item_to_remove]
    if (args.size() != 2 || !args[0].isList()) {
        ReportRuntimeError("List.remove(item) expects a list and the item to remove.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    auto& list = args[0].asListMutable();
    const MegaladonValue& item_to_remove = args[1];

    auto it = std::remove_if(list.begin(), list.end(), [&](const MegaladonValue& val) {
        return val == item_to_remove;
    });

    bool removed = (it != list.end());
    list.erase(it, list.end());
    return MegaladonValue(removed); // Return true if at least one item was removed, false otherwise
}

// Helper for sort: custom comparison for MegaladonValue (can be expanded)
bool compareMegaladonValues(const MegaladonValue& a, const MegaladonValue& b) {
    if (a.isNumber() && b.isNumber()) {
        return a.asNumber() < b.asNumber();
    }
    if (a.isString() && b.isString()) {
        return a.asString() < b.asString();
    }
    // For other types or mixed types, define a specific order or throw error
    throw std::runtime_error("Cannot compare values of different or unsupported types for sorting.");
}

MegaladonValue list_sort(const std::vector<MegaladonValue>& args) {
    // Expects: [this_list]
    if (args.size() != 1 || !args[0].isList()) {
        ReportRuntimeError("List.sort() expects a list.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    auto& list = args[0].asListMutable(); // Modify in place
    try {
        std::sort(list.begin(), list.end(), compareMegaladonValues);
    } catch (const std::runtime_error& e) {
        ReportRuntimeError("Error during list sort: " + std::string(e.what()));
        return MegaladonValue(MegaladonValue::INVALID);
    }
    return MegaladonValue(); // Return void, as it sorts in place
}

MegaladonValue list_filter(Interpreter& interpreter, const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, filter_function]
    if (args.size() != 2 || !args[0].isList() || !args[1].isFunction()) {
        ReportRuntimeError("List.filter(func) expects a list and a function.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const auto& original_list = args[0].asList();
    MegaladonCallable* filter_func = args[1].asFunction();

    std::vector<MegaladonValue> filtered_list;
    for (const auto& item : original_list) {
        // Call the filter function with the current item
        std::vector<MegaladonValue> func_args = {item};
        if (filter_func->arity() != 1) {
             ReportRuntimeError("Filter function must accept exactly one argument.");
             return MegaladonValue(MegaladonValue::INVALID);
        }
        MegaladonValue result = filter_func->call(interpreter, func_args);
        if (result.isTruthy()) {
            filtered_list.push_back(item);
        }
    }
    return MegaladonValue(filtered_list);
}

MegaladonValue list_map(Interpreter& interpreter, const std::vector<MegaladonValue>& args) {
    // Expects: [this_list, map_function]
    if (args.size() != 2 || !args[0].isList() || !args[1].isFunction()) {
        ReportRuntimeError("List.map(func) expects a list and a function.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const auto& original_list = args[0].asList();
    MegaladonCallable* map_func = args[1].asFunction();

    std::vector<MegaladonValue> mapped_list;
    for (const auto& item : original_list) {
        // Call the map function with the current item
        std::vector<MegaladonValue> func_args = {item};
        if (map_func->arity() != 1) {
            ReportRuntimeError("Map function must accept exactly one argument.");
            return MegaladonValue(MegaladonValue::INVALID);
        }
        MegaladonValue result = map_func->call(interpreter, func_args);
        mapped_list.push_back(result);
    }
    return MegaladonValue(mapped_list);
}