#pragma once

#include <stdio.h>

#include "print_utils.h"
#include "segment_array.h"
#include "token.h"
#include "type.h"
#include "bytecode.h"

// IMPORTANT: the types are below the unions to make pointer casting easy
// DO NOT REARRANGE
// I don't know how well this works with strict aliasing, but that's okay, because I have it disabled

typedef struct ExprNode ExprNode;
typedef struct DeclNode DeclNode;
typedef struct TypeNode TypeNode;
typedef struct Node Node;

typedef enum {
    EXPR_NODE_INT,
    EXPR_NODE_FLOAT,
    EXPR_NODE_BOOL,
    EXPR_NODE_CHAR,
    EXPR_NODE_VAR,
    EXPR_NODE_UNARY_OP,
    EXPR_NODE_BIN_OP,
    EXPR_NODE_LOGICAL_OP,
    EXPR_NODE_CAST,
    EXPR_NODE_ASSIGNMENT,
    EXPR_NODE_BLOCK,
} ExprNodeType;

typedef struct {
    Token token;
    union {
        i64 integer;
        f64 floating;
        bool boolean;
        char character;
    } as;
} LiteralNode;

typedef struct {
    Token token;
    size_t stack_index;
} VarNode;

typedef enum {
    UNARY_NEGATE,
    UNARY_BIT_NOT,
    UNARY_NOT,
} UnaryOpType;

typedef struct {
    UnaryOpType type;
    Token token;
    ExprNode* operand;
} UnaryOpNode;

typedef enum {
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
    BIN_MOD,
    BIN_BIT_AND,
    BIN_BIT_XOR,
    BIN_BIT_OR,
    BIN_BIT_LSHIFT,
    BIN_BIT_RSHIFT,
    BIN_EQ,
    BIN_NE,
    BIN_LT,
    BIN_LE,
    BIN_GT,
    BIN_GE,
} BinOpType;

typedef struct {
    BinOpType type;
    Token token;
    ExprNode* left;
    ExprNode* right;
} BinOpNode;

typedef enum {
    LOG_AND,
    LOG_OR,
} LogicalOpType;

typedef struct {
    LogicalOpType type;
    Token token;
    ExprNode* left;
    ExprNode* right;
} LogicalOpNode;

typedef struct {
    Token token;
    ExprNode* expr;
    TypeNode* type;
    Opcode cast_opcode;
} CastNode;

typedef struct {
    Token token;
    ExprNode* left;
    ExprNode* right;
} AssignmentNode;

typedef struct {
    DynamicArray(Node*) nodes;
    Token rbrace;
    u16 locals_count;
    bool has_value;
} BlockNode;

struct ExprNode {
    union {
        LiteralNode literal;
        VarNode var;
        UnaryOpNode unary_op;
        BinOpNode bin_op;
        LogicalOpNode logical_op;
        CastNode cast;
        AssignmentNode assignment;
        BlockNode block;
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
    TYPE_NODE_PRIMITIVE,
    TYPE_NODE_MONOSTATE,
} TypeNodeType;

typedef struct {
    Token token;
    PrimitiveTypeType type;
} PrimitiveTypeNode;

typedef struct {
    Token token;
} MonostateTypeNode;

struct TypeNode {
    union {
        PrimitiveTypeNode primitive_type;
        MonostateTypeNode monostate_type;
    } as;
    TypeNodeType type;
};

typedef enum {
    NODE_EXPR,
    NODE_DECL,
    NODE_TYPE,
} NodeType;

struct Node {
    union {
        ExprNode expr;
        DeclNode decl;
        TypeNode type;
    } as;
    NodeType type;
};

typedef DynamicArray(Node*) Dyna_Node;

// Might possibly change this later so Program contains Dyna_Node instead of being an alias of it (probably won't though)
typedef Dyna_Node Program;

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
