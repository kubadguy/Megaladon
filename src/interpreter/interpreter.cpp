// src/interpreter/interpreter.cpp
#include "interpreter.h"
#include "../util/error.h" // For runtime error reporting
#include <iostream>
#include <algorithm> // For std::transform for string methods (if not in value.cpp)
#include <utility>   // For std::move

// --- MegaladonFunction Implementation ---
MegaladonValue MegaladonFunction::call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) {
    // Create a new environment for the function call
    // This environment's enclosing scope is the *closure* (where the function was defined)
    std::shared_ptr<Environment> functionEnvironment = std::make_shared<Environment>(closure);

    for (size_t i = 0; i < declaration.params.size(); ++i) {
        functionEnvironment->define(declaration.params[i].lexeme, arguments[i]);
    }

    try {
        interpreter.executeBlock(declaration.body->statements, functionEnvironment);
    } catch (const Interpreter::ReturnValue& return_value) {
        return return_value.value;
    }

    return MegaladonValue(); // Implicit void return
}

// --- Interpreter Class Implementation ---
Interpreter::Interpreter() {
    globalEnvironment = std::make_shared<Environment>();
    currentEnvironment = globalEnvironment;

    // Register built-in functions
    registerBuiltins(globalEnvironment);
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
    try {
        for (const auto& stmt : statements) {
            execute(*stmt);
        }
    } catch (const std::runtime_error& e) {
        ReportRuntimeError(e.what());
    }
}

MegaladonValue Interpreter::evaluate(Expr& expr) {
    return expr.accept(*this);
}

void Interpreter::execute(Stmt& stmt) {
    stmt.accept(*this);
}

bool Interpreter::isTruthy(const MegaladonValue& value) {
    return value.isTruthy();
}

void Interpreter::checkNumberOperand(const Token& op, const MegaladonValue& operand) {
    if (!operand.isNumber()) {
        throw std::runtime_error("Operand of '" + op.lexeme + "' must be a number.");
    }
}

void Interpreter::checkNumberOperands(const Token& op, const MegaladonValue& left, const MegaladonValue& right) {
    if (!left.isNumber() || !right.isNumber()) {
        throw std::runtime_error("Operands of '" + op.lexeme + "' must be numbers.");
    }
}


// --- Expression Visitors ---

MegaladonValue Interpreter::visit(BinaryExpr& expr) {
    MegaladonValue left = evaluate(*expr.left);
    MegaladonValue right = evaluate(*expr.right);

    switch (expr.op.type) {
        case PLUS: return left + right;
        case MINUS: return left - right;
        case ASTERISK: return left * right;
        case SLASH: return left / right;
        case PERCENT: return left % right;

        case GREATER: return MegaladonValue(left > right);
        case GREATER_EQUAL: return MegaladonValue(left >= right);
        case LESS: return MegaladonValue(left < right);
        case LESS_EQUAL: return MegaladonValue(left <= right);
        case BANG_EQUAL: return MegaladonValue(left != right);
        case EQUAL_EQUAL: return MegaladonValue(left == right);
        default:
            break; // Should not happen
    }
    return MegaladonValue(MegaladonValue::INVALID); // Unreachable
}

MegaladonValue Interpreter::visit(GroupingExpr& expr) {
    return evaluate(*expr.expression);
}

MegaladonValue Interpreter::visit(LiteralExpr& expr) {
    return expr.value;
}

MegaladonValue Interpreter::visit(UnaryExpr& expr) {
    MegaladonValue right = evaluate(*expr.right);

    switch (expr.op.type) {
        case MINUS:
            checkNumberOperand(expr.op, right);
            return MegaladonValue(-right.asNumber());
        case BANG:
            return MegaladonValue(!isTruthy(right));
        default:
            break; // Should not happen
    }
    return MegaladonValue(MegaladonValue::INVALID); // Unreachable
}

MegaladonValue Interpreter::visit(VariableExpr& expr) {
    return currentEnvironment->get(expr.name.lexeme);
}

MegaladonValue Interpreter::visit(AssignExpr& expr) {
    MegaladonValue value = evaluate(*expr.value);
    currentEnvironment->assign(expr.name.lexeme, value);
    return value; // Assignment expressions evaluate to the assigned value
}

MegaladonValue Interpreter::visit(LogicalExpr& expr) {
    MegaladonValue left = evaluate(*expr.left);

    if (expr.op.type == OR) {
        if (isTruthy(left)) return left;
    } else { // AND
        if (!isTruthy(left)) return left;
    }

    return evaluate(*expr.right);
}

MegaladonValue Interpreter::visit(CallExpr& expr) {
    MegaladonValue callee = evaluate(*expr.callee);

    std::vector<MegaladonValue> arguments;
    for (const auto& arg_expr : expr.arguments) {
        arguments.push_back(evaluate(*arg_expr));
    }

    if (!callee.isFunction()) {
        throw std::runtime_error("Can only call functions and built-ins.");
    }

    MegaladonCallable* function = callee.asFunction();
    if (arguments.size() != function->arity()) {
        throw std::runtime_error("Expected " + std::to_string(function->arity()) +
                                 " arguments but got " + std::to_string(arguments.size()) + ".");
    }

    return function->call(*this, arguments);
}

MegaladonValue Interpreter::visit(GetExpr& expr) {
    MegaladonValue object = evaluate(*expr.object);

    // This is where object.method() calls are handled.
    // Megaladon currently has methods for primitive types (string, list).
    // Future: add support for user-defined objects.
    if (object.isString() || object.isList()) {
        // Dispatch to the MegaladonValue's callMethod for built-in methods
        return object.callMethod(*this, expr.name.lexeme, {}); // Pass interpreter for higher-order functions
    }

    // For now, if it's not a string or list, it doesn't have properties/methods.
    throw std::runtime_error("Cannot get property '" + expr.name.lexeme + "' on value of type '" + object.toString() + "'.");
}

MegaladonValue Interpreter::visit(SetExpr& expr) {
    // This assumes object.property = value syntax which is not yet fully supported
    // for user-defined objects. For now, it will only apply if `GetExpr` can somehow
    // be a direct assignable reference, which it isn't currently for primitives.
    // This will require more complex object model.
    // For now, only list[index] = value is handled via IndexAssignExpr.
    throw std::runtime_error("Property assignment ('object.property = value') is not directly supported yet for this type. Use list[index] = value for lists.");
    // Example: (If objects were implemented)
    // MegaladonValue object = evaluate(*expr.object);
    // if (object.isObject()) { // Assuming an 'isObject' method and a map inside
    //     object.asObjectMutable().set(expr.name.lexeme, evaluate(*expr.value));
    //     return evaluate(*expr.value);
    // }
    // throw std::runtime_error("Cannot set property on non-object type.");
}

MegaladonValue Interpreter::visit(ListExpr& expr) {
    std::vector<MegaladonValue> elements;
    for (const auto& element_expr : expr.elements) {
        elements.push_back(evaluate(*element_expr));
    }
    return MegaladonValue(elements);
}

MegaladonValue Interpreter::visit(IndexExpr& expr) {
    MegaladonValue object = evaluate(*expr.object);
    MegaladonValue index_val = evaluate(*expr.index);

    if (!index_val.isNumber()) {
        throw std::runtime_error("List/String index must be a number.");
    }

    int index = static_cast<int>(index_val.asNumber());

    if (object.isList()) {
        const auto& list = object.asList();
        if (index < 0 || index >= list.size()) {
            throw std::runtime_error("List index out of bounds: " + std::to_string(index));
        }
        return list[index];
    } else if (object.isString()) {
        const auto& str = object.asString();
        if (index < 0 || index >= str.length()) {
            throw std::runtime_error("String index out of bounds: " + std::to_string(index));
        }
        return MegaladonValue(std::string(1, str[index]));
    }
    throw std::runtime_error("Cannot index non-list or non-string type.");
}

MegaladonValue Interpreter::visit(IndexAssignExpr& expr) {
    MegaladonValue object = evaluate(*expr.object);
    MegaladonValue index_val = evaluate(*expr.index);
    MegaladonValue value_to_assign = evaluate(*expr.value);

    if (!index_val.isNumber()) {
        throw std::runtime_error("List index must be a number for assignment.");
    }

    int index = static_cast<int>(index_val.asNumber());

    if (object.isList()) {
        auto& list = object.asListMutable(); // Get mutable reference to modify in place
        if (index < 0 || index >= list.size()) {
            throw std::runtime_error("List index out of bounds for assignment: " + std::to_string(index));
        }
        list[index] = value_to_assign;
        return value_to_assign;
    }
    // String indexing assignment is not standard (strings are immutable in many languages).
    // If you need string modification, provide a .replace() method.
    throw std::runtime_error("Cannot assign to index of non-list type (strings are immutable).");
}


// --- Statement Visitors ---

void Interpreter::visit(ExpressionStmt& stmt) {
    evaluate(*stmt.expression);
}

void Interpreter::visit(PrintStmt& stmt) {
    MegaladonValue value = evaluate(*stmt.expression);
    std::cout << value.toString() << std::endl;
}

void Interpreter::visit(VarDeclStmt& stmt) {
    MegaladonValue value;
    if (stmt.initializer) {
        value = evaluate(*stmt.initializer);
    } else {
        value = MegaladonValue(); // Default to void if no initializer
    }
    currentEnvironment->define(stmt.name.lexeme, value);
}

void Interpreter::visit(BlockStmt& stmt) {
    // Create a new scope for the block and execute its statements within it.
    executeBlock(stmt.statements, std::make_shared<Environment>(currentEnvironment));
}

void Interpreter::executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment) {
    std::shared_ptr<Environment> previous = currentEnvironment;
    try {
        currentEnvironment = environment;
        for (const auto& stmt : statements) {
            execute(*stmt);
        }
    } catch (...) {
        // Ensure environment is restored even if an error/return occurs
        currentEnvironment = previous;
        throw; // Re-throw the exception
    }
    currentEnvironment = previous;
}

void Interpreter::visit(IfStmt& stmt) {
    if (isTruthy(evaluate(*stmt.condition))) {
        execute(*stmt.thenBranch);
    } else if (stmt.elseBranch) {
        execute(*stmt.elseBranch);
    }
}

void Interpreter::visit(WhileStmt& stmt) {
    while (isTruthy(evaluate(*stmt.condition))) {
        execute(*stmt.body);
    }
}

void Interpreter::visit(FunctionStmt& stmt) {
    // Wrap the AST function declaration in our callable object
    // Using std::shared_ptr for MegaladonCallable to manage lifetime
    std::shared_ptr<MegaladonCallable> function_ptr = std::make_shared<MegaladonFunction>(stmt, currentEnvironment);
    currentEnvironment->define(stmt.name.lexeme, MegaladonValue(function_ptr));
}

void Interpreter::visit(ReturnStmt& stmt) {
    MegaladonValue value;
    if (stmt.value) {
        value = evaluate(*stmt.value);
    }
    throw ReturnValue(value); // Throw an exception to unwind stack for return
}