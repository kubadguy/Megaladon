#include "builtins.h"
#include "../types/value.h"
#include "../util/error.h"
#include <algorithm> // For std::sort

// Helper to check if a value is an integer number
bool isInteger(const MegaladonValue& val) {
    return val.isNumber() && (std::fmod(val.asNumber(), 1.0) == 0.0);
}

// --- List Built-in Functions ---

// list.add(item)
// Takes the list as arguments[0] and the item to add as arguments[1]
// The first argument (the list itself) must be mutable.
MegaladonValue list_add(std::vector<MegaladonValue>& arguments) { // Removed const
    if (arguments.size() != 2 || !arguments[0].isList()) {
        throw std::runtime_error("MegaladonError: list.add() expects a list object and one argument.");
    }
    // Now arguments[0] is mutable
    arguments[0].asListMutable().push_back(arguments[1]);
    return MegaladonValue(); // Return VOID for in-place modification
}

// list.remove_at(index)
MegaladonValue list_remove_at(std::vector<MegaladonValue>& arguments) { // Removed const
    if (arguments.size() != 2 || !arguments[0].isList() || !isInteger(arguments[1])) {
        throw std::runtime_error("MegaladonError: list.remove_at() expects a list object and an integer index.");
    }

    auto& list = arguments[0].asListMutable();
    int index = static_cast<int>(arguments[1].asNumber());

    if (index < 0 || static_cast<size_t>(index) >= list.size()) {
        throw std::runtime_error("MegaladonError: List index out of bounds in remove_at.");
    }

    MegaladonValue removed_item = list[index];
    list.erase(list.begin() + index);
    return removed_item;
}

// list.get(index)
MegaladonValue list_get(const std::vector<MegaladonValue>& arguments) {
    if (arguments.size() != 2 || !arguments[0].isList() || !isInteger(arguments[1])) {
        throw std::runtime_error("MegaladonError: list.get() expects a list object and an integer index.");
    }

    const auto& list = arguments[0].asList();
    int index = static_cast<int>(arguments[1].asNumber());

    if (index < 0 || static_cast<size_t>(index) >= list.size()) {
        throw std::runtime_error("MegaladonError: List index out of bounds in get.");
    }

    return list[index];
}

// list.set(index, value)
MegaladonValue list_set(std::vector<MegaladonValue>& arguments) { // Removed const
    if (arguments.size() != 3 || !arguments[0].isList() || !isInteger(arguments[1])) {
        throw std::runtime_error("MegaladonError: list.set() expects a list object, an integer index, and a value.");
    }

    auto& list = arguments[0].asListMutable();
    int index = static_cast<int>(arguments[1].asNumber());
    const MegaladonValue& value = arguments[2];

    if (index < 0 || static_cast<size_t>(index) >= list.size()) {
        throw std::runtime_error("MegaladonError: List index out of bounds in set.");
    }

    list[index] = value;
    return MegaladonValue(); // Return VOID
}

// list.insert_at(index, value)
MegaladonValue list_insert_at(std::vector<MegaladonValue>& arguments) { // Removed const
    if (arguments.size() != 3 || !arguments[0].isList() || !isInteger(arguments[1])) {
        throw std::runtime_error("MegaladonError: list.insert_at() expects a list object, an integer index, and a value.");
    }

    auto& list = arguments[0].asListMutable();
    int index = static_cast<int>(arguments[1].asNumber());
    const MegaladonValue& value = arguments[2];

    // Allow inserting at list.size() for append-like behavior
    if (index < 0 || static_cast<size_t>(index) > list.size()) {
        throw std::runtime_error("MegaladonError: List index out of bounds in insert_at.");
    }

    list.insert(list.begin() + index, value);
    return MegaladonValue(); // Return VOID
}

// list.pop(index?) - removes and returns the last element, or at index if provided
MegaladonValue list_pop(std::vector<MegaladonValue>& arguments) { // Removed const
    if (arguments.size() < 1 || arguments.size() > 2 || !arguments[0].isList()) {
        throw std::runtime_error("MegaladonError: list.pop() expects a list object and an optional integer index.");
    }

    auto& list = arguments[0].asListMutable();

    if (list.empty()) {
        throw std::runtime_error("MegaladonError: Cannot pop from an empty list.");
    }

    int index_to_pop;
    if (arguments.size() == 1) {
        index_to_pop = static_cast<int>(list.size() - 1); // Last element
    } else { // arguments.size() == 2
        if (!isInteger(arguments[1])) {
            throw std::runtime_error("MegaladonError: list.pop() index must be an integer.");
        }
        index_to_pop = static_cast<int>(arguments[1].asNumber());
    }

    if (index_to_pop < 0 || static_cast<size_t>(index_to_pop) >= list.size()) {
        throw std::runtime_error("MegaladonError: List index out of bounds in pop.");
    }

    MegaladonValue popped_value = list[index_to_pop];
    list.erase(list.begin() + index_to_pop);
    return popped_value;
}

// list.clear()
MegaladonValue list_clear(std::vector<MegaladonValue>& arguments) { // Removed const
    if (arguments.size() != 1 || !arguments[0].isList()) {
        throw std::runtime_error("MegaladonError: list.clear() expects a list object.");
    }
    arguments[0].asListMutable().clear();
    return MegaladonValue(); // Return VOID
}

// list.size() (or length - might prefer global 'len' for consistency)
// Not implemented as a method, assuming global 'len' for lists.
// If you want it as a method:
/*
MegaladonValue list_size(const std::vector<MegaladonValue>& arguments) {
    if (arguments.size() != 1 || !arguments[0].isList()) {
        throw std::runtime_error("MegaladonError: list.size() expects a list object.");
    }
    return MegaladonValue(static_cast<double>(arguments[0].asList().size()));
}
*/

// list.remove(value) - removes first occurrence of value
MegaladonValue list_remove(std::vector<MegaladonValue>& arguments) { // Removed const
    if (arguments.size() != 2 || !arguments[0].isList()) {
        throw std::runtime_error("MegaladonError: list.remove() expects a list object and a value to remove.");
    }

    auto& list = arguments[0].asListMutable();
    const MegaladonValue& value_to_remove = arguments[1];

    auto it = std::remove(list.begin(), list.end(), value_to_remove);
    if (it == list.end()) {
        throw std::runtime_error("MegaladonError: Value not found in list for remove.");
    }
    list.erase(it, list.end());
    return MegaladonValue(); // Return VOID
}

// Comparator for sorting MegaladonValues (simple numeric/string comparison)
bool megaladonValueLessThan(const MegaladonValue& a, const MegaladonValue& b) {
    if (a.isNumber() && b.isNumber()) {
        return a.asNumber() < b.asNumber();
    }
    if (a.isString() && b.isString()) {
        return a.asString() < b.asString();
    }
    // Define behavior for other types or throw an error for unsupported comparisons
    throw std::runtime_error("MegaladonError: Cannot compare these types for sorting.");
}

// list.sort()
MegaladonValue list_sort(std::vector<MegaladonValue>& arguments) { // Removed const
    if (arguments.size() != 1 || !arguments[0].isList()) {
        throw std::runtime_error("MegaladonError: list.sort() expects a list object.");
    }

    auto& list = arguments[0].asListMutable(); // Modify in place
    if (!list.empty()) {
        // Basic check if elements are comparable before sorting
        ValueType first_type = list[0].type;
        bool all_same_type = true;
        for (size_t i = 1; i < list.size(); ++i) {
            if (list[i].type != first_type || (!list[i].isNumber() && !list[i].isString())) {
                all_same_type = false;
                break;
            }
        }

        if (!all_same_type) {
            throw std::runtime_error("MegaladonError: List contains incomparable types for sorting.");
        }

        std::sort(list.begin(), list.end(), megaladonValueLessThan);
    }
    return MegaladonValue(); // Return VOID
}

// --- Register List Methods ---
// This part would typically be in core_function.cpp or a similar central registration point
// It demonstrates how you'd define methods accessible via dot notation on a List type in Megaladon.
// This requires your interpreter to handle method calls (e.g., list.add(item)).

/*
// Example of how to register list methods if you have a way to define methods on types
void registerListMethods(std::shared_ptr<Environment>& env) {
    // This part is more complex as it depends on how you represent classes/objects
    // For now, assuming direct built-in functions that take the list as the first arg.
    // If you have a MegaladonList class that encapsulates std::vector<MegaladonValue>,
    // its methods would look different.
}
*/