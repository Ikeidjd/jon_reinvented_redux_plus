#include "semantic_analyzer.h"
#include "semantic_analyzer_internals.h"

#include <stdio.h>
#include <stdarg.h>

bool analyze_program(Program* program) {
    SemanticAnalyzer self;
    sema_init(&self);

    bool success = true;

    for (size_t i = 0; i < program->length; i++) {
        if (!analyze_node(&self, program->data[i])) success = false;
    }

    return success;
}

static TypeAndMutability from_type(Type* type, bool mutable) {
    return (TypeAndMutability) {
        .type = type,
        .mutable = mutable,
    };
}

static void sema_init(SemanticAnalyzer* self) {
    type_arena_init(&self->arena);
    dyna_init(&self->vars.vars);
    self->vars.depth = 0;
}

static void sema_add_var(SemanticAnalyzer* self, StringView name, TypeAndMutability type) {
    Var var = (Var) {
        .name = name,
        .type = type,
        .depth = self->vars.depth,
    };

    dyna_append(&self->vars.vars, var);
}

static bool sema_get_var(SemanticAnalyzer* self, StringView name, Var* out) {
    for (size_t i = self->vars.vars.length; i--;) {
        Var var = self->vars.vars.data[i];

        if (sv_eq(name, var.name)) {
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
    (void) self;

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
            Var var;
            if (!sema_get_var(self, expr->as.var.token.text, &var)) {
                sema_error_undeclared_variable(self, expr->as.var.token);
                return false;
            }

            *out = var.type;
            return true;
        }
        case EXPR_NODE_BIN_OP: {
            TypeAndMutability left;
            if (!analyze_expr(self, expr->as.bin_op.left, &left)) return false;

            if (expr->as.bin_op.type == BIN_ASSIGN && !left.mutable) {
                sema_error_assign_to_immutable(self, expr->as.bin_op.token);
                return false;
            }

            TypeAndMutability right;
            if (!analyze_expr(self, expr->as.bin_op.right, &right)) return false;

            if (left.type != right.type) {
                sema_error_incompatible_types(self, expr->as.bin_op.token, left.type, right.type);
                return false;
            }

            *out = left;
            return false;
        }
    }

    return false;
}

static bool analyze_decl(SemanticAnalyzer* self, DeclNode* decl) {
    (void) self;

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

            sema_add_var(self, decl->as.var_decl.var_name.text, (TypeAndMutability) {
                .type = expr_type.type,
                .mutable = decl->as.var_decl.mutable,
            });
        } break;
    }

    return false;
}

static Type* analyze_type(SemanticAnalyzer* self, TypeNode* type) {
    (void) self;

    switch (type->type) {
        case TYPE_NODE_BASIC: {
            return (Type*) basic_type_new(&self->arena, type->as.basic_type.type);
        }
    }

    return false;
}
