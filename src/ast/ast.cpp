#include "ast.h"
#include "../interpreter/interpreter.h" // For Interpreter (ExprVisitor, StmtVisitor concrete implementation)

// --- Expression accept methods ---
MegaladonValue AssignExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<AssignExpr>(shared_from_this()));
}

MegaladonValue BinaryExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<BinaryExpr>(shared_from_this()));
}

MegaladonValue CallExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<CallExpr>(shared_from_this()));
}

MegaladonValue GetExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<GetExpr>(shared_from_this()));
}

MegaladonValue GroupingExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<GroupingExpr>(shared_from_this()));
}

MegaladonValue LiteralExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<LiteralExpr>(shared_from_this()));
}

MegaladonValue LogicalExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<LogicalExpr>(shared_from_this()));
}

MegaladonValue SetExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<SetExpr>(shared_from_this()));
}

MegaladonValue UnaryExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<UnaryExpr>(shared_from_this()));
}

MegaladonValue VariableExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<VariableExpr>(shared_from_this()));
}

MegaladonValue ListExpr::accept(ExprVisitor<MegaladonValue>& visitor) {
    return visitor.visit(std::static_pointer_cast<ListExpr>(shared_from_this()));
}


// --- Statement accept methods ---
void BlockStmt::accept(StmtVisitor<void>& visitor) {
    visitor.visit(std::static_pointer_cast<BlockStmt>(shared_from_this()));
}

void ExpressionStmt::accept(StmtVisitor<void>& visitor) {
    visitor.visit(std::static_pointer_cast<ExpressionStmt>(shared_from_this()));
}

void FunctionStmt::accept(StmtVisitor<void>& visitor) {
    visitor.visit(std::static_pointer_cast<FunctionStmt>(shared_from_this()));
}

void IfStmt::accept(StmtVisitor<void>& visitor) {
    visitor.visit(std::static_pointer_cast<IfStmt>(shared_from_this()));
}

void PrintStmt::accept(StmtVisitor<void>& visitor) {
    visitor.visit(std::static_pointer_cast<PrintStmt>(shared_from_this()));
}

void ReturnStmt::accept(StmtVisitor<void>& visitor) {
    visitor.visit(std::static_pointer_cast<ReturnStmt>(shared_from_this()));
}

void VarStmt::accept(StmtVisitor<void>& visitor) {
    visitor.visit(std::static_pointer_cast<VarStmt>(shared_from_this()));
}

void WhileStmt::accept(StmtVisitor<void>& visitor) {
    visitor.visit(std::static_pointer_cast<WhileStmt>(shared_from_this()));
}