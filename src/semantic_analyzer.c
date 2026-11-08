#include "semantic_analyzer.h"
#include "semantic_analyzer_internals.h"

#include <stdio.h>
#include <stdarg.h>

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
    type_print(left);
    fprintf(stderr, " and ");
    type_print(right);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);
}

static void sema_error_invalid_types_for_op(SemanticAnalyzer* self, Token op, Type* left, Type* right) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: operation '"SV_FMT"' can not be performed on ", op.line_number + 1, SV_ARG(op.text));
    type_print(left);
    fprintf(stderr, " and ");
    type_print(right);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, op);
}

static void sema_error_invalid_type_for_op(SemanticAnalyzer* self, Token op, Type* type) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: operation '"SV_FMT"' can not be performed on ", op.line_number + 1, SV_ARG(op.text));
    type_print(type);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, op);
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

static void sema_error_too_many_vars(SemanticAnalyzer* self, Token token) {
    (void) self;

    fprintf(stderr, "Semantic error on line %zu: reached variable limit of %d.\n", token.line_number + 1, VARS_CAPACITY);
    token_show_in_line(stderr, token);
}

static bool analyze_node(SemanticAnalyzer* self, Node* node) {
    switch (node->type) {
        case NODE_EXPR: {
            TypeAndMutability _;
            return analyze_expr(self, &node->as.expr, &_);
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
    switch (expr->type) {
        case EXPR_NODE_INT: {
            *out = from_type((Type*) basic_type_new(&self->arena, BASIC_TYPE_INT), false);
            return true;
        }
        case EXPR_NODE_FLOAT: {
            *out = from_type((Type*) basic_type_new(&self->arena, BASIC_TYPE_FLOAT), false);
            return true;
        }
        case EXPR_NODE_BOOL: {
            *out = from_type((Type*) basic_type_new(&self->arena, BASIC_TYPE_BOOL), false);
            return true;
        }
        case EXPR_NODE_CHAR: {
            *out = from_type((Type*) basic_type_new(&self->arena, BASIC_TYPE_CHAR), false);
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
        case EXPR_NODE_BIN_OP: {
            TypeAndMutability left;
            if (!analyze_expr(self, expr->as.bin_op.left, &left)) return false;

            TypeAndMutability right;
            if (!analyze_expr(self, expr->as.bin_op.right, &right)) return false;

            Type* int_type = (Type*) basic_type_new(&self->arena, BASIC_TYPE_INT);
            Type* float_type = (Type*) basic_type_new(&self->arena, BASIC_TYPE_FLOAT);

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
            }

            if (left.type != right.type) {
                sema_error_incompatible_types(self, expr->as.bin_op.token, left.type, right.type);
                return false;
            }

            *out = left;
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
        case EXPR_NODE_UNARY_OP: {
            TypeAndMutability operand;
            if (!analyze_expr(self, expr->as.unary_op.operand, &operand)) return false;

            Type* int_type = (Type*) basic_type_new(&self->arena, BASIC_TYPE_INT);
            Type* float_type = (Type*) basic_type_new(&self->arena, BASIC_TYPE_FLOAT);

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
            }

            *out = operand;
            return true;
        }
    }

    return false;
}

static bool analyze_decl(SemanticAnalyzer* self, DeclNode* decl) {
    switch (decl->type) {
        case DECL_NODE_VAR: {
            Type* type = NULL;
            if (decl->as.var_decl.type) {
                type = analyze_type(self, decl->as.var_decl.type);
                if (!type) return false;
            }

            TypeAndMutability expr_type;
            if (!analyze_expr(self, decl->as.var_decl.expr, &expr_type)) return false;

            if (type && type != expr_type.type) {
                sema_error_incompatible_types(self, decl->as.var_decl.equals_sign, type, expr_type.type);
                return false;
            }

            bool success = sema_add_var(self, decl->as.var_decl.var_name.text, (TypeAndMutability) {
                .type = expr_type.type,
                .mutable = decl->as.var_decl.mutable,
            });

            if (!success) {
                sema_error_too_many_vars(self, decl->as.var_decl.equals_sign);
            }

            return success;
        }
    }

    return false;
}

static Type* analyze_type(SemanticAnalyzer* self, TypeNode* type) {
    switch (type->type) {
        case TYPE_NODE_BASIC: {
            return (Type*) basic_type_new(&self->arena, type->as.basic_type.type);
        }
    }

    return NULL;
}
