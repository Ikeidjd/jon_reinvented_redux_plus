#pragma once

#include <stdio.h>

#include "print_utils.h"
#include "segment_array.h"
#include "token.h"
#include "type.h"

// IMPORTANT: the types are below the unions to make pointer casting easy
// DO NOT REARRANGE
// I don't know how well this works with strict aliasing, but that's okay, because I have it disabled

typedef struct ExprNode ExprNode;
typedef struct DeclNode DeclNode;
typedef struct TypeNode TypeNode;

typedef enum {
    EXPR_NODE_INT,
    EXPR_NODE_FLOAT,
    EXPR_NODE_BOOL,
    EXPR_NODE_CHAR,
    EXPR_NODE_VAR,
    EXPR_NODE_BIN_OP,
} ExprNodeType;

typedef struct {
    Token token;
} LiteralNode;

typedef struct {
    Token token;
} VarNode;

typedef enum {
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
    BIN_MOD,
    BIN_ASSIGN,
} BinOpType;

typedef struct {
    BinOpType type;
    Token token;
    ExprNode* left;
    ExprNode* right;
} BinOpNode;

struct ExprNode {
    union {
        LiteralNode literal;
        VarNode var;
        BinOpNode bin_op;
    } as;
    ExprNodeType type;
};

typedef enum {
    DECL_NODE_VAR,
} DeclNodeType;

typedef struct {
    Token var_name;
    Token equals_sign; // For debug information
    TypeNode* type;
    ExprNode* expr;
    bool mutable;
} VarDeclNode;

struct DeclNode {
    union {
        VarDeclNode var_decl;
    } as;
    DeclNodeType type;
};

typedef enum {
    TYPE_NODE_BASIC,
} TypeNodeType;

typedef struct {
    Token token;
    BasicTypeType type;
} BasicTypeNode;

struct TypeNode {
    union {
        BasicTypeNode basic_type;
    } as;
    TypeNodeType type;
};

typedef enum {
    NODE_EXPR,
    NODE_DECL,
    NODE_TYPE,
} NodeType;

typedef struct {
    union {
        ExprNode expr;
        DeclNode decl;
        TypeNode type;
    } as;
    NodeType type;
} Node;

typedef DynamicArray(Node*) Program;

void expr_node_fprintln(FILE* file, const ExprNode* expr_node);

PRINTLN_GEN(const ExprNode*, expr_node)

void decl_node_fprintln(FILE* file, const DeclNode* decl_node);

PRINTLN_GEN(const DeclNode*, decl_node)

void type_node_fprintln(FILE* file, const TypeNode* type_node);

PRINTLN_GEN(const TypeNode*, type_node)

void node_fprintln(FILE* file, const Node* node);

PRINTLN_GEN(const Node*, node)

void program_fprintln(FILE* file, Program program);

PRINTLN_GEN(Program, program)

typedef SegmentArray(Node) Sega_Node;
