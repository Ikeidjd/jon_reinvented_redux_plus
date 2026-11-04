#pragma once

#include <stdio.h>

#include "print_utils.h"
#include "segment_array.h"
#include "token.h"

// IMPORTANT: the types are below the unions to make pointer casting easy
// DO NOT REARRANGE
// I don't know how well this works with strict aliasing, but that's okay, because I have it disabled

typedef enum {
    EXPR_INT,
    EXPR_VAR,
    EXPR_BIN_OP,
} NodeExprType;

typedef struct NodeExpr NodeExpr;

typedef struct {
    Token token;
} NodeInt;

typedef struct {
    Token token;
} NodeVar;

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
    NodeExpr* left;
    NodeExpr* right;
} NodeBinOp;

struct NodeExpr {
    union {
        NodeInt integer;
        NodeVar var;
        NodeBinOp bin_op;
    } as;
    NodeExprType type;
};

typedef enum {
    DECL_VAR,
} NodeDeclType;

typedef struct {
    Token var_name;
    Token equals_sign; // For debug information
    NodeExpr* expr;
} NodeVarDecl;

typedef struct {
    union {
        NodeVarDecl var_decl;
    } as;
    NodeDeclType type;
} NodeDecl;

typedef enum {
    NODE_EXPR,
    NODE_DECL,
} NodeType;

typedef struct {
    union {
        NodeExpr expr;
        NodeDecl decl;
    } as;
    NodeType type;
} Node;

typedef DynamicArray(Node*) Program;

void node_expr_fprintln(FILE* file, const NodeExpr* node_expr);

PRINTLN_GEN(const NodeExpr*, node_expr)

void node_decl_fprintln(FILE* file, const NodeDecl* node_decl);

PRINTLN_GEN(const NodeDecl*, node_decl)

void node_fprintln(FILE* file, const Node* node);

PRINTLN_GEN(const Node*, node)

void program_fprintln(FILE* file, Program program);

PRINTLN_GEN(Program, program)

typedef SegmentArray(Node) Sega_Node;
