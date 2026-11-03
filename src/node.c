#include "node.h"

static void node_fprintln_indent(FILE* file, const Node* node, size_t indent) {
    for (size_t i = 0; i < indent; i++) {
        fputc('-', file);
    }

    indent += 2;

    switch (node->type) {
        case NODE_INT: {
            fprintf(file, "Int("SV_FMT")\n", SV_ARG(node->as.integer.token.text));
        } break;
        case NODE_BIN_OP: {
            fprintf(file, "BinOp("SV_FMT"):\n", SV_ARG(node->as.bin_op.token.text));
            node_fprintln_indent(file, node->as.bin_op.left, indent);
            node_fprintln_indent(file, node->as.bin_op.right, indent);
        } break;
    }
}

void node_fprintln(FILE* file, const Node* node) {
    node_fprintln_indent(file, node, 0);
}
