#pragma once

#include "node.h"
#include "bytecode.h"

typedef struct {
    Chunk chunk;
} Compiler;

static void compiler_init(Compiler* self);

static void compile_node(Compiler* self, Node* node);

static void compile_expr(Compiler* self, ExprNode* expr);

static void compile_expr_lvalue(Compiler* self, ExprNode* expr);

static void compile_decl(Compiler* self, DeclNode* decl);

static void compile_type(Compiler* self, TypeNode* type);
