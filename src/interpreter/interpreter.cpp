#include "interpreter.h"
#include "../builtins/builtins.h" // For registerBuiltins
#include "../util/error.h"
#include <iostream>
#include <string> // For std::stod

// Constructor
Interpreter::Interpreter() {
    globals = std::make_shared<Environment>();
    environment = globals; // Current environment starts as global

    // Register built-in functions
    registerBuiltins(globals);
}

// Main interpretation loop
void Interpreter::interpret(const std::vector<std::shared_ptr<Stmt>>& statements) {
    try {
        for (const auto& statement : statements) {
            execute(statement);
        }
    } catch (const MegaladonError& e) {
        // You would typically report this error to the user
        std::cerr << "Runtime Error: " << e.what() << "\n";
        // Optionally, reset state or exit
    } catch (const std::runtime_error& e) {
        std::cerr << "Internal Runtime Error: " << e.what() << "\n";
    }
}

// Execute a statement
void Interpreter::execute(std::shared_ptr<Stmt> stmt) {
    stmt->accept(*this);
}

// Evaluate an expression
MegaladonValue Interpreter::evaluate(std::shared_ptr<Expr> expr) {
    return expr->accept(*this);
}

void Interpreter::executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> new_environment) {
    std::shared_ptr<Environment> previous = this->environment;
    try {
        this->environment = new_environment;
        for (const auto& statement : statements) {
            execute(statement);
        }
    } catch (const ReturnValue& r) {
        this->environment = previous; // Restore previous environment on return
        throw; // Re-throw the return value
    }
    this->environment = previous; // Restore previous environment
}

// Helper for number operand checks
void Interpreter::checkNumberOperand(const Token& op, const MegaladonValue& operand) {
    if (!operand.isNumber()) {
        throw MegaladonError(op, "Operand must be a number.");
    }
}

void Interpreter::checkNumberOperands(const Token& op, const MegaladonValue& left, const MegaladonValue& right) {
    if (!left.isNumber() || !right.isNumber()) {
        throw MegaladonError(op, "Operands must be numbers.");
    }
}

bool Interpreter::isTruthy(const MegaladonValue& value) {
    if (value.isVoid()) return false;
    if (value.isBoolean()) return value.asBoolean();
    return true; // Any other value (numbers, strings, lists) is true
}

bool Interpreter::isEqual(const MegaladonValue& a, const MegaladonValue& b) {
    // Rely on MegaladonValue's overloaded operator==
    return a == b;
}

// --- Expression Visitors ---

MegaladonValue Interpreter::visit(std::shared_ptr<AssignExpr> expr) {
    MegaladonValue value = evaluate(expr->value);

    // If a resolution distance is set, use it
    if (expr->distance != -1) {
        environment->assignAt(expr->distance, expr->name, value);
    } else {
        // Otherwise, assume global or dynamically resolved (if no resolver)
        globals->assign(expr->name, value);
    }
    return value;
}


MegaladonValue Interpreter::visit(std::shared_ptr<BinaryExpr> expr) {
    MegaladonValue left = evaluate(expr->left);
    MegaladonValue right = evaluate(expr->right);

    switch (expr->op.type) {
        case TokenType::GREATER:
            checkNumberOperands(expr->op, left, right);
            return MegaladonValue(left.asNumber() > right.asNumber());
        case TokenType::GREATER_EQUAL:
            checkNumberOperands(expr->op, left, right);
            return MegaladonValue(left.asNumber() >= right.asNumber());
        case TokenType::LESS:
            checkNumberOperands(expr->op, left, right);
            return MegaladonValue(left.asNumber() < right.asNumber());
        case TokenType::LESS_EQUAL:
            checkNumberOperands(expr->op, left, right);
            return MegaladonValue(left.asNumber() <= right.asNumber());
        case TokenType::MINUS:
            checkNumberOperands(expr->op, left, right);
            return MegaladonValue(left.asNumber() - right.asNumber());
        case TokenType::PLUS:
            if (left.isNumber() && right.isNumber()) {
                return MegaladonValue(left.asNumber() + right.asNumber());
            }
            if (left.isString() && right.isString()) {
                return MegaladonValue(left.asString() + right.asString());
            }
            if (left.isList() && right.isList()) {
                std::vector<MegaladonValue> newList = left.asList();
                const auto& rightList = right.asList();
                newList.insert(newList.end(), rightList.begin(), rightList.end());
                return MegaladonValue(newList);
            }
            throw MegaladonError(expr->op, "Operands must be two numbers, two strings, or two lists.");
        case TokenType::SLASH:
            checkNumberOperands(expr->op, left, right);
            if (right.asNumber() == 0) {
                throw MegaladonError(expr->op, "Division by zero.");
            }
            return MegaladonValue(left.asNumber() / right.asNumber());
        case TokenType::STAR:
            checkNumberOperands(expr->op, left, right);
            return MegaladonValue(left.asNumber() * right.asNumber());
        case TokenType::BANG_EQUAL:
            return MegaladonValue(!isEqual(left, right));
        case TokenType::EQUAL_EQUAL:
            return MegaladonValue(isEqual(left, right));
        default:
            // Should not happen if parser is correct
            return MegaladonValue(ValueType::INVALID);
    }
}

MegaladonValue Interpreter::visit(std::shared_ptr<CallExpr> expr) {
    MegaladonValue callee = evaluate(expr->callee);

    std::vector<MegaladonValue> arguments;
    for (const auto& arg : expr->arguments) {
        arguments.push_back(evaluate(arg));
    }

    if (!callee.isFunction()) {
        throw MegaladonError(expr->paren, "Can only call functions.");
    }

    std::shared_ptr<MegaladonCallable> function = callee.asCallable();

    if (function->arity() != arguments.size() && function->arity() != -1) { // -1 for variable arity
        throw MegaladonError(expr->paren, "Expected " + std::to_string(function->arity()) +
                                       " arguments but got " + std::to_string(arguments.size()) + ".");
    }

    return function->call(*this, arguments);
}


MegaladonValue Interpreter::visit(std::shared_ptr<GroupingExpr> expr) {
    return evaluate(expr->expression);
}

MegaladonValue Interpreter::visit(std::shared_ptr<LiteralExpr> expr) {
    // LiteralExpr already holds MegaladonValue
    return expr->value;
}

MegaladonValue Interpreter::visit(std::shared_ptr<LogicalExpr> expr) {
    MegaladonValue left = evaluate(expr->left);

    if (expr->op.type == TokenType::OR) {
        if (isTruthy(left)) return left;
    } else { // AND
        if (!isTruthy(left)) return left;
    }

    return evaluate(expr->right);
}

MegaladonValue Interpreter::visit(std::shared_ptr<UnaryExpr> expr) {
    MegaladonValue right = evaluate(expr->right);

    switch (expr->op.type) {
        case TokenType::BANG:
            return MegaladonValue(!isTruthy(right));
        case TokenType::MINUS:
            checkNumberOperand(expr->op, right);
            return MegaladonValue(-right.asNumber());
        default:
            // Should not happen
            return MegaladonValue(ValueType::INVALID);
    }
}

MegaladonValue Interpreter::visit(std::shared_ptr<VariableExpr> expr) {
    if (expr->distance != -1) {
        return environment->getAt(expr->distance, expr->name.lexeme);
    } else {
        // Fallback to global if not resolved (e.g., built-ins)
        return globals->get(expr->name);
    }
}

MegaladonValue Interpreter::visit(std::shared_ptr<ListExpr> expr) {
    std::vector<MegaladonValue> elements;
    for (const auto& item_expr : expr->elements) {
        elements.push_back(evaluate(item_expr));
    }
    return MegaladonValue(elements);
}

MegaladonValue Interpreter::visit(std::shared_ptr<GetExpr> expr) {
    MegaladonValue object = evaluate(expr->object);

    if (object.isList()) {
        // This is for list indexing, e.g., myList[0]
        if (!expr->index || !expr->index->isLiteral()) { // Assuming index is a literal for now
             throw MegaladonError(expr->name, "List index must be a literal number.");
        }
        MegaladonValue index_value = evaluate(expr->index);
        if (!index_value.isNumber() || std::fmod(index_value.asNumber(), 1.0) != 0.0) {
            throw MegaladonError(expr->name, "List index must be an integer.");
        }

        int index = static_cast<int>(index_value.asNumber());
        const auto& list = object.asList();

        if (index < 0 || static_cast<size_t>(index) >= list.size()) {
            throw MegaladonError(expr->name, "List index out of bounds.");
        }
        return list[index];
    }
    // Handle other object properties if Megaladon supports them (e.g., object.property)
    throw MegaladonError(expr->name, "Only lists support indexed access.");
}

MegaladonValue Interpreter::visit(std::shared_ptr<SetExpr> expr) {
    MegaladonValue object = evaluate(expr->object);
    MegaladonValue value_to_set = evaluate(expr->value);

    if (object.isList()) {
        if (!expr->index || !expr->index->isLiteral()) { // Assuming index is a literal for now
            throw MegaladonError(expr->name, "List index must be a literal number for assignment.");
        }
        MegaladonValue index_value = evaluate(expr->index);
        if (!index_value.isNumber() || std::fmod(index_value.asNumber(), 1.0) != 0.0) {
            throw MegaladonError(expr->name, "List index for assignment must be an integer.");
        }

        int index = static_cast<int>(index_value.asNumber());
        auto& list = object.asListMutable(); // Need mutable access

        if (index < 0 || static_cast<size_t>(index) >= list.size()) {
            throw MegaladonError(expr->name, "List index out of bounds for assignment.");
        }
        list[index] = value_to_set;
        return value_to_set;
    }
    throw MegaladonError(expr->name, "Only lists support indexed assignment.");
}


// --- Statement Visitors ---

void Interpreter::visit(std::shared_ptr<ExpressionStmt> stmt) {
    evaluate(stmt->expression);
}

void Interpreter::visit(std::shared_ptr<PrintStmt> stmt) {
    MegaladonValue value = evaluate(stmt->expression);
    std::cout << value.toString() << "\n";
}

void Interpreter::visit(std::shared_ptr<VarStmt> stmt) {
    MegaladonValue value;
    if (stmt->initializer) {
        value = evaluate(stmt->initializer);
    } else {
        value = MegaladonValue(); // Default to VOID
    }
    environment->define(stmt->name.lexeme, value);
}

void Interpreter::visit(std::shared_ptr<BlockStmt> stmt) {
    // Create a new environment for the block
    executeBlock(stmt->statements, std::make_shared<Environment>(this->environment));
}

void Interpreter::visit(std::shared_ptr<IfStmt> stmt) {
    if (isTruthy(evaluate(stmt->condition))) {
        execute(stmt->thenBranch);
    } else if (stmt->elseBranch) {
        execute(stmt->elseBranch);
    }
}

void Interpreter::visit(std::shared_ptr<WhileStmt> stmt) {
    while (isTruthy(evaluate(stmt->condition))) {
        execute(stmt->body);
    }
}

// Represents a user-defined function as a MegaladonCallable
class MegaladonFunction : public MegaladonCallable {
public:
    MegaladonFunction(std::shared_ptr<FunctionStmt> declaration, std::shared_ptr<Environment> closure)
        : declaration(std::move(declaration)), closure(std::move(closure)) {}

    int arity() const override { return static_cast<int>(declaration->params.size()); }
    std::string toString() const override { return "<fn " + declaration->name.lexeme + ">"; }

    MegaladonValue call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) override {
        // Create a new environment for the function's body
        std::shared_ptr<Environment> function_environment = std::make_shared<Environment>(closure);

        // Bind arguments to parameters in the new environment
        for (size_t i = 0; i < declaration->params.size(); ++i) {
            function_environment->define(declaration->params[i].lexeme, arguments[i]);
        }

        try {
            interpreter.executeBlock(declaration->body->statements, function_environment);
        } catch (const ReturnValue& returnValue) {
            return returnValue.value;
        }

        return MegaladonValue(); // Implicit return VOID
    }

private:
    std::shared_ptr<FunctionStmt> declaration;
    std::shared_ptr<Environment> closure; // Environment where the function was defined
};


void Interpreter::visit(std::shared_ptr<FunctionStmt> stmt) {
    // When a function declaration is evaluated, it becomes a Callable object.
    // The current environment becomes the function's closure.
    std::shared_ptr<MegaladonFunction> function = std::make_shared<MegaladonFunction>(stmt, environment);
    environment->define(stmt->name.lexeme, MegaladonValue(function));
}

void Interpreter::visit(std::shared_ptr<ReturnStmt> stmt) {
    MegaladonValue value;
    if (stmt->value) {
        value = evaluate(stmt->value);
    } else {
        value = MegaladonValue(); // Return VOID by default
    }
    throw ReturnValue(value);
}