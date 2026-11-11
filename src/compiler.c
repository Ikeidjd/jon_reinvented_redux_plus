#include "compiler.h"
#include "compiler_internals.h"

#include <assert.h>

#include "little_endian_utils.h"

void compile_program(Program program, Chunk* chunk) {
    Compiler self = {
        .chunk = *chunk,
    };

    for (size_t i = 0; i < program.length; i++) {
        compile_node(&self, program.data[i]);
    }

    *chunk = self.chunk;
}

static void compile_node(Compiler* self, Node* node) {
    switch (node->type) {
        case NODE_EXPR: {
            compile_expr(self, &node->as.expr);
            chunk_emit(&self->chunk, OP_POP, self->chunk.lines.data[self->chunk.lines.length - 1]);
        } break;
        case NODE_STAT: {
            compile_stat(self, &node->as.stat);
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

static Opcode logical_op_type_to_jump(LogicalOpType type) {
    switch (type) {
        case LOG_AND:
            return OP_JUMP_IF_FALSE;
        case LOG_OR:
            return OP_JUMP_IF_TRUE;
    }

    assert(false && "Unreachable");
}

static Opcode unary_op_type_to_opcode(UnaryOpType type) {
    switch (type) {
        case UNARY_NEGATE:
            return OP_NEGATE;
        case UNARY_BIT_NOT:
            return OP_BIT_NOT;
        case UNARY_NOT:
            return OP_NOT;
    }

    assert(false && "Unreachable");
}

static void compile_expr(Compiler* self, ExprNode* expr) {
    switch (expr->type) {
        case EXPR_NODE_INT: {
            chunk_emit_load_int(&self->chunk, expr->as.literal.as.integer, expr->as.literal.token.line_number);
        } break;
        case EXPR_NODE_FLOAT: {
            chunk_emit_load_float(&self->chunk, expr->as.literal.as.floating, expr->as.literal.token.line_number);
        } break;
        case EXPR_NODE_BOOL: {
            chunk_emit_load_bool(&self->chunk, expr->as.literal.as.boolean, expr->as.literal.token.line_number);
        } break;
        case EXPR_NODE_CHAR: {
            chunk_emit_load_char(&self->chunk, expr->as.literal.as.character, expr->as.literal.token.line_number);
        } break;
        case EXPR_NODE_VAR: {
            chunk_emit_get_local(&self->chunk, expr->as.var.stack_index, expr->as.var.token.line_number);
        } break;
        case EXPR_NODE_UNARY_OP: {
            compile_expr(self, expr->as.unary_op.operand);
            chunk_emit(&self->chunk, unary_op_type_to_opcode(expr->as.unary_op.type), expr->as.unary_op.token.line_number);
        } break;
        case EXPR_NODE_BIN_OP: {
            compile_expr(self, expr->as.bin_op.left);
            compile_expr(self, expr->as.bin_op.right);
            chunk_emit(&self->chunk, bin_op_type_to_opcode(expr->as.bin_op.type), expr->as.bin_op.token.line_number);
        } break;
        case EXPR_NODE_LOGICAL_OP: {
            compile_expr(self, expr->as.logical_op.left);
            chunk_emit(&self->chunk, OP_DUP, expr->as.logical_op.token.line_number);

            size_t jump_op_index = chunk_emit_jump(&self->chunk, logical_op_type_to_jump(expr->as.logical_op.type), expr->as.logical_op.token.line_number);

            chunk_emit(&self->chunk, OP_POP, expr->as.logical_op.token.line_number);
            compile_expr(self, expr->as.logical_op.right);

            chunk_patch_jump(&self->chunk, jump_op_index);
        } break;
        case EXPR_NODE_CAST: {
            compile_expr(self, expr->as.cast.expr);
            chunk_emit(&self->chunk, expr->as.cast.cast_opcode, expr->as.cast.token.line_number);
        } break;
        case EXPR_NODE_ASSIGNMENT: {
            compile_expr(self, expr->as.assignment.right);
            compile_expr_lvalue(self, expr->as.assignment.left, expr->as.assignment.token.line_number);
        } break;
        case EXPR_NODE_BLOCK: {
            for (size_t i = 0; i < expr->as.block.nodes.length; i++) {
                Node* node = expr->as.block.nodes.data[i];

                if (i == expr->as.block.nodes.length - 1 && expr->as.block.has_value) {
                    assert(expr->as.block.nodes.data[i]->type == NODE_EXPR);
                    compile_expr(self, &node->as.expr);
                } else {
                    compile_node(self, node);
                }
            }

            if (!expr->as.block.has_value) {
                chunk_emit(&self->chunk, OP_LOAD_MONOSTATE, expr->as.block.rbrace.line_number);
            }

            u8 buf[2];
            to_le_u16(expr->as.block.locals_count, buf);
            chunk_emit_args(&self->chunk, OP_END_BLOCK, 2, buf, expr->as.block.rbrace.line_number);
        } break;
        case EXPR_NODE_IF_ELSE: {
            compile_expr(self, expr->as.if_else.condition);
            size_t jump_to_else = chunk_emit_jump(&self->chunk, OP_JUMP_IF_FALSE, expr->as.if_else.token.line_number);

            compile_expr(self, (ExprNode*) expr->as.if_else.if_branch);
            size_t jump_to_end = chunk_emit_jump(&self->chunk, OP_JUMP, self->chunk.lines.data[self->chunk.lines.length - 1]);

            chunk_patch_jump(&self->chunk, jump_to_else);

            if (expr->as.if_else.else_branch) {
                compile_expr(self, expr->as.if_else.else_branch);
            } else {
                chunk_emit(&self->chunk, OP_LOAD_MONOSTATE, self->chunk.lines.data[self->chunk.lines.length - 1]);
            }

            chunk_patch_jump(&self->chunk, jump_to_end);
        } break;
    }
}

static void compile_expr_lvalue(Compiler* self, ExprNode* expr, size_t line) {
    assert(expr->type == EXPR_NODE_VAR);
    chunk_emit_set_local(&self->chunk, expr->as.var.stack_index, line);
}

static void compile_stat(Compiler* self, StatNode* stat) {
    switch (stat->type) {
        case STAT_NODE_WHILE_LOOP: {
            size_t pos_before_condition = self->chunk.bytes.length;

            compile_expr(self, stat->as.while_loop.condition);
            size_t condition_jump = chunk_emit_jump(&self->chunk, OP_JUMP_IF_FALSE, stat->as.while_loop.token.line_number);

            compile_expr(self, (ExprNode*) stat->as.while_loop.body);
            chunk_emit(&self->chunk, OP_POP, stat->as.while_loop.token.line_number); // Pop the value that was pushed by the loop block (always a monostate value)
            chunk_emit_loop(&self->chunk, pos_before_condition, stat->as.while_loop.token.line_number);

            chunk_patch_jump(&self->chunk, condition_jump);
        } break;
    }
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
