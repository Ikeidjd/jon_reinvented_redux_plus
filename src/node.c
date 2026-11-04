#include "node.h"

#define INDENT_INCREMENT 2

static void process_indent(FILE* file, size_t* indent) {
    for (size_t i = 0; i < *indent; i++) {
        fputc('-', file);
    }

    *indent = *indent + INDENT_INCREMENT;
}

static void node_expr_fprintln_indent(FILE* file, const NodeExpr* node_expr, size_t indent) {
    process_indent(file, &indent);

    switch (node_expr->type) {
        case EXPR_INT: {
            fprintf(file, "Int("SV_FMT")\n", SV_ARG(node_expr->as.integer.token.text));
        } break;
        case EXPR_VAR: {
            fprintf(file, "Var("SV_FMT")\n", SV_ARG(node_expr->as.var.token.text));
        } break;
        case EXPR_BIN_OP: {
            fprintf(file, "BinOp("SV_FMT"):\n", SV_ARG(node_expr->as.bin_op.token.text));
            node_expr_fprintln_indent(file, node_expr->as.bin_op.left, indent);
            node_expr_fprintln_indent(file, node_expr->as.bin_op.right, indent);
        } break;
    }
}

static void node_decl_fprintln_indent(FILE* file, const NodeDecl* node_decl, size_t indent) {
    process_indent(file, &indent);

    switch (node_decl->type) {
        case DECL_VAR: {
            fprintf(file, "Let("SV_FMT"):\n", SV_ARG(node_decl->as.var_decl.var_name.text));
            node_expr_fprintln_indent(file, node_decl->as.var_decl.expr, indent);
        } break;
    }
}

static void node_fprintln_indent(FILE* file, const Node* node, size_t indent) {
    switch (node->type) {
        case NODE_EXPR: {
            node_expr_fprintln_indent(file, &node->as.expr, indent);
        } break;
        case NODE_DECL: {
            node_decl_fprintln_indent(file, &node->as.decl, indent);
        } break;
    }
}

void node_expr_fprintln(FILE* file, const NodeExpr* node_expr) {
    node_expr_fprintln_indent(file, node_expr, 0);
}

void node_decl_fprintln(FILE* file, const NodeDecl* node_decl) {
    node_decl_fprintln_indent(file, node_decl, 0);
}

void node_fprintln(FILE* file, const Node* node) {
    node_fprintln_indent(file, node, 0);
}

void program_fprintln(FILE* file, Program program) {
    fprintf(file, "Program\n");
    for (size_t i = 0; i < program.length; i++) {
        node_fprintln_indent(file, program.data[i], INDENT_INCREMENT);
    }
}
