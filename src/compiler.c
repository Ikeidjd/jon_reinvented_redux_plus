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

static Opcode bin_op_type_to_opcode(BinOpType type) {
    switch (type) {
        case BIN_ADD:
            return OP_ADD;
        case BIN_SUB:
            return OP_SUB;
        case BIN_MUL:
            return OP_MUL;
        case BIN_DIV:
            return OP_DIV;
        case BIN_MOD:
            return OP_MOD;
        case BIN_BIT_AND:
            return OP_BIT_AND;
        case BIN_BIT_XOR:
            return OP_BIT_XOR;
        case BIN_BIT_OR:
            return OP_BIT_OR;
        case BIN_BIT_LSHIFT:
            return OP_BIT_LSHIFT;
        case BIN_BIT_RSHIFT:
            return OP_BIT_RSHIFT;
        case BIN_EQ:
            return OP_EQ;
        case BIN_NE:
            return OP_NE;
        case BIN_LT:
            return OP_LT;
        case BIN_LE:
            return OP_LE;
        case BIN_GT:
            return OP_GT;
        case BIN_GE:
            return OP_GE;
    }

    assert(false && "Unreachable");
}

static Opcode unary_op_type_to_opcode(UnaryOpType type) {
    switch (type) {
        case UNARY_NEGATE:
            return OP_NEGATE;
        case UNARY_BIT_NOT:
            return OP_BIT_NOT;
    }

    assert(false && "Unreachable");
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
            chunk_emit(&self->chunk, bin_op_type_to_opcode(expr->as.bin_op.type));
        } break;
        case EXPR_NODE_ASSIGNMENT: {
            compile_expr(self, expr->as.assignment.right);
            compile_expr_lvalue(self, expr->as.assignment.left);
        } break;
        case EXPR_NODE_UNARY_OP: {
            compile_expr(self, expr->as.unary_op.operand);
            chunk_emit(&self->chunk, unary_op_type_to_opcode(expr->as.unary_op.type));
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
