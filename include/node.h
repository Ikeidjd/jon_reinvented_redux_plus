#pragma once

#include <stdio.h>

#include "print_utils.h"
#include "segment_array.h"
#include "token.h"

typedef enum {
    NODE_INT,
    NODE_BIN_OP,
} NodeType;

typedef struct Node Node;

typedef struct {
    Token token;
} NodeInt;

typedef enum {
    BIN_ADD,
    BIN_SUB,
    BIN_MUL,
    BIN_DIV,
} BinOpType;

typedef struct {
    BinOpType type;
    Token token;
    Node* left;
    Node* right;
} NodeBinOp;

struct Node {
    NodeType type;
    union {
        NodeInt integer;
        NodeBinOp bin_op;
    } as;
};

void node_fprintln(FILE* file, const Node* node);

PRINTLN_GEN(Node, node)

typedef SegmentArray(Node) Sega_Node;
