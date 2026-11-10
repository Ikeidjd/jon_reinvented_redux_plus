#pragma once

#include "node.h"
#include "bytecode.h"

typedef struct {
    Chunk chunk;
} Compiler;

static void compile_node(Compiler* self, Node* node);

static Opcode bin_op_type_to_opcode(BinOpType type);

static Opcode logical_op_type_to_jump(LogicalOpType type);

static Opcode unary_op_type_to_opcode(UnaryOpType type);

static void compile_expr(Compiler* self, ExprNode* expr);

static void compile_expr_lvalue(Compiler* self, ExprNode* expr, size_t line);

static void compile_decl(Compiler* self, DeclNode* decl);

static void compile_type(Compiler* self, TypeNode* type);
