#include "compiler.h"
#include "compiler_internals.h"

#include <assert.h>

Chunk compile_program(Program program) {
    Compiler self;
    compiler_init(&self);

    for (size_t i = 0; i < program.length; i++) {
        compile_node(&self, program.data[i]);
    }

    return self.chunk;
}

static void compiler_init(Compiler* self) {
    dyna_init(&self->chunk);
}

static void compile_node(Compiler* self, Node* node) {
    switch (node->type) {
        case NODE_EXPR: {
            compile_expr(self, &node->as.expr);
            chunk_emit(&self->chunk, OP_POP);
        } break;
        case NODE_DECL: {
            compile_decl(self, &node->as.decl);
        } break;
        case NODE_TYPE: {
            compile_type(self, &node->as.type);
        } break;
    }
}

static void compile_expr(Compiler* self, ExprNode* expr) {
    switch (expr->type) {
        case EXPR_NODE_INT: {
            chunk_emit_load_int(&self->chunk, expr->as.literal.as.integer);
        } break;
        case EXPR_NODE_FLOAT: {
            chunk_emit_load_float(&self->chunk, expr->as.literal.as.floating);
        } break;
        case EXPR_NODE_BOOL: {
            chunk_emit_load_bool(&self->chunk, expr->as.literal.as.boolean);
        } break;
        case EXPR_NODE_CHAR: {
            chunk_emit_load_char(&self->chunk, expr->as.literal.as.character);
        } break;
        case EXPR_NODE_VAR: {
            chunk_emit_get_local(&self->chunk, expr->as.var.stack_index);
        } break;
        case EXPR_NODE_BIN_OP: {
            compile_expr(self, expr->as.bin_op.left);
            compile_expr(self, expr->as.bin_op.right);

            Opcode op;
            switch (expr->as.bin_op.type) {
                case BIN_ADD: {
                    op = OP_ADD;
                } break;
                case BIN_SUB: {
                    op = OP_SUB;
                } break;
                case BIN_MUL: {
                    op = OP_MUL;
                } break;
                case BIN_DIV: {
                    op = OP_DIV;
                } break;
                case BIN_MOD: {
                    op = OP_MOD;
                } break;
            }

            chunk_emit(&self->chunk, op);
        } break;
        case EXPR_NODE_ASSIGNMENT: {
            compile_expr(self, expr->as.assignment.right);
            compile_expr_lvalue(self, expr->as.assignment.left);
        } break;
        case EXPR_NODE_UNARY_OP: {
            compile_expr(self, expr->as.unary_op.operand);

            Opcode op;
            switch (expr->as.unary_op.type) {
                case UNARY_MINUS: {
                    op = OP_NEGATE;
                } break;
            }

            chunk_emit(&self->chunk, op);
        } break;
    }
}

static void compile_expr_lvalue(Compiler* self, ExprNode* expr) {
    assert(expr->type == EXPR_NODE_VAR);
    chunk_emit_set_local(&self->chunk, expr->as.var.stack_index);
}

static void compile_decl(Compiler* self, DeclNode* decl) {
    switch (decl->type) {
        case DECL_NODE_VAR: {
            compile_type(self, decl->as.var_decl.type);
            compile_expr(self, decl->as.var_decl.expr);
        } break;
    }
}

static void compile_type(Compiler* self, TypeNode* type) {
    (void) self;
    (void) type;
    // nop
    // Maybe I could use this for runtime type information later?
    // I'll just pretend this isn't an empty function for now and call it when needed
}
