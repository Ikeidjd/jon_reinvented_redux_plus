#pragma once

typedef struct {
    Type* type;
    bool mutable;
} TypeAndMutability;

static TypeAndMutability from_type(Type* type, bool mutable);

typedef struct {
    StringView name;
    TypeAndMutability type;
    size_t depth;
} Var;

#define VARS_CAPACITY 65536

typedef struct {
    Var data[VARS_CAPACITY];
    size_t top;
    size_t depth;
} Vars;

// Does not own arena
typedef struct {
    TypeArena arena;
    Vars vars;
} SemanticAnalyzer;

static void sema_init(SemanticAnalyzer* self, TypeArena arena);

static void sema_start_block(SemanticAnalyzer* self);

static size_t sema_end_block(SemanticAnalyzer* self);

static bool sema_add_var(SemanticAnalyzer* self, StringView name, TypeAndMutability type);

static bool sema_get_var(SemanticAnalyzer* self, StringView name, size_t* out_index, Var* out);

static void sema_error(SemanticAnalyzer* self, Token token, const char* fmt, ...) __attribute__((format(printf, 3, 4)));

static void sema_error_incompatible_types(SemanticAnalyzer* self, Token token, Type* left, Type* right);

static void sema_error_invalid_types_for_op(SemanticAnalyzer* self, Token token, Type* left, Type* right);

static void sema_error_invalid_type_for_op(SemanticAnalyzer* self, Token token, Type* type);

static void sema_error_undeclared_variable(SemanticAnalyzer* self, Token token);

static void sema_error_assign_to_immutable(SemanticAnalyzer* self, Token token);

static void sema_error_casting_non_primitives(SemanticAnalyzer* self, Token token, Type* src, Type* dst);

static void sema_error_casting(SemanticAnalyzer* self, Token token, PrimitiveType* src, PrimitiveType* dst);

static void sema_error_non_bool_for_if_condition(SemanticAnalyzer* self, Token token, Type* type);

static void sema_error_incompatible_types_for_if_branches(SemanticAnalyzer* self, Token token, Type* if_type, Type* else_type);

static void sema_error_no_else_branch_for_non_monostate_if(SemanticAnalyzer* self, Token token, Type* if_type);

static void sema_error_too_many_vars(SemanticAnalyzer* self, Token token);

static bool analyze_node(SemanticAnalyzer* self, Node* node);

static bool analyze_expr(SemanticAnalyzer* self, ExprNode* expr, TypeAndMutability* out);

static bool analyze_decl(SemanticAnalyzer* self, DeclNode* decl);

static Type* analyze_type(SemanticAnalyzer* self, TypeNode* type);
