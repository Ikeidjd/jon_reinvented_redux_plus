#include "semantic_analyzer.h"
#include "semantic_analyzer_internals.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

bool analyze_program(Program* program, TypeArena* arena) {
    SemanticAnalyzer* self = malloc(sizeof(*self));
    sema_init(self, *arena);

    bool success = true;

    for (size_t i = 0; i < program->length; i++) {
        if (!analyze_node(self, program->data[i])) {
            success = false;
        }
    }

    *arena = self->arena;
    free(self);
    return success;
}

static TypeAndMutability from_type(Type* type, bool mutable) {
    return (TypeAndMutability) {
        .type = type,
        .mutable = mutable,
    };
}

static void sema_init(SemanticAnalyzer* self, TypeArena arena) {
    self->arena = arena;
    self->vars.top = 0;
    self->vars.depth = 0;
}

static void sema_start_block(SemanticAnalyzer* self) {
    self->vars.depth++;
}

static size_t sema_end_block(SemanticAnalyzer* self) {
    size_t locals_count = 0;

    for (size_t i = self->vars.top; i-- && self->vars.data[i].depth == self->vars.depth;) {
        locals_count++;
    }

    self->vars.depth--;

    self->vars.top -= locals_count;
    return locals_count;
}

static bool sema_add_var(SemanticAnalyzer* self, StringView name, TypeAndMutability type) {
    Var var = {
        .name = name,
        .type = type,
        .depth = self->vars.depth,
    };

    if (self->vars.top >= VARS_CAPACITY) return false;

    self->vars.data[self->vars.top++] = var;
    return true;
}

static bool sema_get_var(SemanticAnalyzer* self, StringView name, size_t* out_index, Var* out) {
    for (size_t i = self->vars.top; i--;) {
        Var var = self->vars.data[i];

        if (sv_eq(name, var.name)) {
            *out_index = i;
            *out = var;
            return true;
        }
    }

    return false;
}

static void sema_error(SemanticAnalyzer* self, Token token, const char* fmt, ...) {
    (void) self;

    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Semantic error on line %zu: ", token.line_number + 1);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);

    va_end(args);
}

static void sema_error_incompatible_types(SemanticAnalyzer* self, Token token, Type* left, Type* right) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: incompatible types ", token.line_number + 1);
    type_fprint(stderr, left);
    fprintf(stderr, " and ");
    type_fprint(stderr, right);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_invalid_types_for_op(SemanticAnalyzer* self, Token token, Type* left, Type* right) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: operation '"SV_FMT"' cannot be performed on ", token.line_number + 1, SV_ARG(token.text));
    type_fprint(stderr, left);
    fprintf(stderr, " and ");
    type_fprint(stderr, right);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_invalid_type_for_op(SemanticAnalyzer* self, Token token, Type* type) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: operation '"SV_FMT"' cannot be performed on ", token.line_number + 1, SV_ARG(token.text));
    type_fprint(stderr, type);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_undeclared_variable(SemanticAnalyzer* self, Token token) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: undeclared variable "SV_FMT".\n", token.line_number + 1, SV_ARG(token.text));
    token_show_in_line(stderr, token);
}

static void sema_error_assign_to_immutable(SemanticAnalyzer* self, Token token) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: cannot assign to immutable.\n", token.line_number + 1);
    token_show_in_line(stderr, token);
}

static void sema_error_casting_non_primitives(SemanticAnalyzer* self, Token token, Type* src, Type* dst) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: casting is only supported between primitive types, not between ", token.line_number + 1);
    type_fprint(stderr, src);
    fprintf(stderr, " and ");
    type_fprint(stderr, dst);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_casting(SemanticAnalyzer* self, Token token, PrimitiveType* src, PrimitiveType* dst) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: cannot cast from ", token.line_number + 1);
    type_fprint(stderr, (Type*) src);
    fprintf(stderr, " to ");
    type_fprint(stderr, (Type*) dst);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_non_bool_for_condition(SemanticAnalyzer* self, Token token, Type* type) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: condition must be bool, but was ", token.line_number + 1);
    type_fprint(stderr, type);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_incompatible_types_for_if_branches(SemanticAnalyzer* self, Token token, Type* if_type, Type* else_type) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: if and else types must be the same, but were ", token.line_number + 1);
    type_fprint(stderr, if_type);
    fprintf(stderr, " and ");
    type_fprint(stderr, else_type);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_no_else_branch_for_non_monostate_if(SemanticAnalyzer* self, Token token, Type* if_type) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: if branch with a value of type ", token.line_number + 1);
    type_fprint(stderr, if_type);
    fprintf(stderr, " must have a corresponding else branch.\n");
    token_show_in_line(stderr, token);
}

static void sema_error_loop_body_returns_value(SemanticAnalyzer* self, Token token, Type* body_type) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: loop body must return type monostate, but returned ", token.line_number + 1);
    type_fprint(stderr, body_type);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_too_many_vars(SemanticAnalyzer* self, Token token) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: reached variable limit of %d.\n", token.line_number + 1, VARS_CAPACITY);
    token_show_in_line(stderr, token);
}

static bool analyze_node(SemanticAnalyzer* self, Node* node) {
    switch (node->type) {
        case NODE_EXPR: {
            return analyze_expr(self, &node->as.expr, NULL);
        }
        case NODE_STAT: {
            return analyze_stat(self, &node->as.stat);
        }
        case NODE_DECL: {
            return analyze_decl(self, &node->as.decl);
        }
        case NODE_TYPE: {
            return analyze_type(self, &node->as.type);
        }
    }

    return false;
}

static bool analyze_expr(SemanticAnalyzer* self, ExprNode* expr, TypeAndMutability* out) {
    TypeAndMutability _;
    if (!out) out = &_;

    *out = from_type(NULL, false);

    switch (expr->type) {
        case EXPR_NODE_INT: {
            *out = from_type((Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_INT), false);
            return true;
        }
        case EXPR_NODE_FLOAT: {
            *out = from_type((Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_FLOAT), false);
            return true;
        }
        case EXPR_NODE_BOOL: {
            *out = from_type((Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_BOOL), false);
            return true;
        }
        case EXPR_NODE_CHAR: {
            *out = from_type((Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_CHAR), false);
            return true;
        }
        case EXPR_NODE_VAR: {
            size_t index;
            Var var;
            if (!sema_get_var(self, expr->as.var.token.text, &index, &var)) {
                sema_error_undeclared_variable(self, expr->as.var.token);
                return false;
            }

            expr->as.var.stack_index = index;
            *out = var.type;
            return true;
        }
        case EXPR_NODE_UNARY_OP: {
            TypeAndMutability operand;
            if (!analyze_expr(self, expr->as.unary_op.operand, &operand)) return false;

            Type* int_type = (Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_INT);
            Type* float_type = (Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_FLOAT);

            switch (expr->as.unary_op.type) {
                case UNARY_NEGATE: {
                    if (operand.type != int_type && operand.type != float_type) {
                        sema_error_invalid_type_for_op(self, expr->as.bin_op.token, operand.type);
                        return false;
                    }
                } break;
                case UNARY_BIT_NOT: {
                    if (operand.type != int_type) {
                        sema_error_invalid_type_for_op(self, expr->as.bin_op.token, operand.type);
                        return false;
                    }
                } break;
                case UNARY_NOT: {
                    if (operand.type != (Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_BOOL)) {
                        sema_error_invalid_type_for_op(self, expr->as.bin_op.token, operand.type);
                        return false;
                    }
                } break;
            }

            *out = operand;
            return true;
        }
        case EXPR_NODE_BIN_OP: {
            TypeAndMutability left;
            if (!analyze_expr(self, expr->as.bin_op.left, &left)) return false;

            TypeAndMutability right;
            if (!analyze_expr(self, expr->as.bin_op.right, &right)) return false;

            if (left.type != right.type) {
                sema_error_incompatible_types(self, expr->as.bin_op.token, left.type, right.type);
                return false;
            }

            Type* int_type = (Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_INT);
            Type* float_type = (Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_FLOAT);

            switch (expr->as.bin_op.type) {
                case BIN_ADD:
                case BIN_SUB:
                case BIN_MUL:
                case BIN_DIV: {
                    if (left.type != int_type && left.type != float_type) {
                        sema_error_invalid_types_for_op(self, expr->as.bin_op.token, left.type, right.type);
                        return false;
                    }
                } break;
                case BIN_MOD:
                case BIN_BIT_AND:
                case BIN_BIT_XOR:
                case BIN_BIT_OR:
                case BIN_BIT_LSHIFT:
                case BIN_BIT_RSHIFT: {
                    if (left.type != int_type) {
                        sema_error_invalid_types_for_op(self, expr->as.bin_op.token, left.type, right.type);
                        return false;
                    }
                } break;
                case BIN_EQ:
                case BIN_NE: {
                    *out = from_type((Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_BOOL), false);
                    return true;
                }
                case BIN_LT:
                case BIN_LE:
                case BIN_GT:
                case BIN_GE: {
                    if (left.type != int_type && left.type != float_type && left.type != ((Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_CHAR))) {
                        sema_error_invalid_types_for_op(self, expr->as.bin_op.token, left.type, right.type);
                        return false;
                    }

                    *out = from_type((Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_BOOL), false);
                    return true;
                }
            }

            *out = left;
            return true;
        }
        case EXPR_NODE_LOGICAL_OP: {
            TypeAndMutability left;
            if (!analyze_expr(self, expr->as.logical_op.left, &left)) return false;

            TypeAndMutability right;
            if (!analyze_expr(self, expr->as.logical_op.right, &right)) return false;

            if (left.type != (Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_BOOL)) {
                sema_error_invalid_types_for_op(self, expr->as.logical_op.token, left.type, right.type);
                return false;
            }

            if (left.type != right.type) {
                sema_error_incompatible_types(self, expr->as.logical_op.token, left.type, right.type);
                return false;
            }

            *out = left;
            return true;
        }
        case EXPR_NODE_CAST: {
            TypeAndMutability src;
            if (!analyze_expr(self, expr->as.cast.expr, &src)) return false;

            Type* dst = analyze_type(self, expr->as.cast.type);
            if (!dst) return false;

            if (!src.type || src.type->type != TYPE_PRIMITIVE || dst->type != TYPE_PRIMITIVE) {
                sema_error_casting_non_primitives(self, expr->as.cast.token, src.type, dst);
                return false;
            }

            PrimitiveType* src_type = (PrimitiveType*) src.type;
            PrimitiveType* dst_type = (PrimitiveType*) dst;

            switch (src_type->type) {
                case PRIMITIVE_TYPE_INT: {
                    if (dst_type->type != PRIMITIVE_TYPE_INT && dst_type->type != PRIMITIVE_TYPE_FLOAT && dst_type->type != PRIMITIVE_TYPE_CHAR) {
                        sema_error_casting(self, expr->as.cast.token, src_type, dst_type);
                        return false;
                    }
                } break;
                case PRIMITIVE_TYPE_FLOAT: {
                    if (dst_type->type != PRIMITIVE_TYPE_INT && dst_type->type != PRIMITIVE_TYPE_FLOAT) {
                        sema_error_casting(self, expr->as.cast.token, src_type, dst_type);
                        return false;
                    }
                } break;
                case PRIMITIVE_TYPE_BOOL: {
                    if (dst_type->type != PRIMITIVE_TYPE_INT && dst_type->type != PRIMITIVE_TYPE_BOOL) {
                        sema_error_casting(self, expr->as.cast.token, src_type, dst_type);
                        return false;
                    }
                } break;
                case PRIMITIVE_TYPE_CHAR: {
                    if (dst_type->type != PRIMITIVE_TYPE_INT && dst_type->type != PRIMITIVE_TYPE_CHAR) {
                        sema_error_casting(self, expr->as.cast.token, src_type, dst_type);
                        return false;
                    }
                } break;
            }

            switch (dst_type->type) {
                case PRIMITIVE_TYPE_INT: {
                    expr->as.cast.cast_opcode = OP_CAST_INT;
                } break;
                case PRIMITIVE_TYPE_FLOAT: {
                    expr->as.cast.cast_opcode = OP_CAST_FLOAT;
                } break;
                case PRIMITIVE_TYPE_BOOL: {
                    expr->as.cast.cast_opcode = OP_CAST_BOOL;
                } break;
                case PRIMITIVE_TYPE_CHAR: {
                    expr->as.cast.cast_opcode = OP_CAST_CHAR;
                } break;
            }

            *out = from_type(dst, false);
            return true;
        }
        case EXPR_NODE_ASSIGNMENT: {
            TypeAndMutability left;
            if (!analyze_expr(self, expr->as.assignment.left, &left)) return false;

            if (!left.mutable) {
                sema_error_assign_to_immutable(self, expr->as.assignment.token);
                return false;
            }

            TypeAndMutability right;
            if (!analyze_expr(self, expr->as.assignment.right, &right)) return false;

            if (left.type != right.type) {
                sema_error_incompatible_types(self, expr->as.assignment.token, left.type, right.type);
                return false;
            }

            *out = left;
            return true;
        }
        case EXPR_NODE_BLOCK: {
            TypeAndMutability type = from_type(monostate_type_new(&self->arena), false);
            bool success = true;

            sema_start_block(self);

            for (size_t i = 0; i < expr->as.block.nodes.length; i++) {
                Node* node = expr->as.block.nodes.data[i];

                // Last element of block might be an expression, in which case it determines the return value
                if (i == expr->as.block.nodes.length - 1 && expr->as.block.has_value) {
                    assert(node->type == NODE_EXPR && "Block with value should have expression as last element");

                    if (!analyze_expr(self, &node->as.expr, &type)) success = false;
                    continue;
                }

                if (!analyze_node(self, node)) success = false;
            }

            expr->as.block.locals_count = sema_end_block(self);

            if (success) *out = type;
            return success;
        }
        case EXPR_NODE_IF_ELSE: {
            TypeAndMutability condition;
            bool success = analyze_expr(self, expr->as.if_else.condition, &condition);

            if (success && condition.type != (Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_BOOL)) {
                sema_error_non_bool_for_condition(self, expr->as.if_else.token, condition.type);
                success = false;
            }

            TypeAndMutability if_branch;
            success &= analyze_expr(self, (ExprNode*) expr->as.if_else.if_branch, &if_branch);

            if (expr->as.if_else.else_branch) {
                TypeAndMutability else_branch;
                success &= analyze_expr(self, expr->as.if_else.else_branch, &else_branch);

                if (if_branch.type != else_branch.type) {
                    sema_error_incompatible_types_for_if_branches(self, expr->as.if_else.token, if_branch.type, else_branch.type);
                    success = false;
                }
            } else if (if_branch.type != monostate_type_new(&self->arena)) {
                sema_error_no_else_branch_for_non_monostate_if(self, expr->as.if_else.token, if_branch.type);
                success = false;
            }

            if (success) *out = if_branch;
            return success;
        }
    }

    assert(false && "Unreachable");
}

static bool analyze_stat(SemanticAnalyzer* self, StatNode* stat) {
    switch (stat->type) {
        case STAT_NODE_WHILE_LOOP: {
            TypeAndMutability condition;
            bool success = analyze_expr(self, stat->as.while_loop.condition, &condition);

            if (success && condition.type != (Type*) primitive_type_new(&self->arena, PRIMITIVE_TYPE_BOOL)) {
                sema_error_non_bool_for_condition(self, stat->as.while_loop.token, condition.type);
                success = false;
            }

            TypeAndMutability body;
            success &= analyze_expr(self, (ExprNode*) stat->as.while_loop.body, &body);

            if (success && body.type != (Type*) monostate_type_new(&self->arena)) {
                sema_error_loop_body_returns_value(self, stat->as.while_loop.token, body.type);
                success = false;
            }

            return success;
        }
        case STAT_NODE_PRINT: {
            return analyze_expr(self, stat->as.print.expr, NULL);
        }
    }

    assert(false && "Unreachable");
}

static bool analyze_decl(SemanticAnalyzer* self, DeclNode* decl) {
    switch (decl->type) {
        case DECL_NODE_VAR: {
            Type* type = NULL;
            if (decl->as.var_decl.type) {
                type = analyze_type(self, decl->as.var_decl.type);
                if (!type) return false;
            }

            TypeAndMutability expr;
            bool expr_success = analyze_expr(self, decl->as.var_decl.expr, &expr);

            if (expr_success && !type) {
                type = expr.type;
            }

            bool compatible_types = !type || type == expr.type;
            if (!compatible_types) {
                sema_error_incompatible_types(self, decl->as.var_decl.equals_sign, type, expr.type);
            }

            bool variable_successfully_added = sema_add_var(self, decl->as.var_decl.var_name.text, (TypeAndMutability) {
                .type = type,
                .mutable = decl->as.var_decl.mutable,
            });

            if (!variable_successfully_added) {
                sema_error_too_many_vars(self, decl->as.var_decl.equals_sign);
            }

            return expr_success && compatible_types && variable_successfully_added;
        }
    }

    return false;
}

static Type* analyze_type(SemanticAnalyzer* self, TypeNode* type) {
    switch (type->type) {
        case TYPE_NODE_PRIMITIVE: {
            return (Type*) primitive_type_new(&self->arena, type->as.primitive_type.type);
        }
        case TYPE_NODE_MONOSTATE: {
            return monostate_type_new(&self->arena);
        }
    }

    return NULL;
}
