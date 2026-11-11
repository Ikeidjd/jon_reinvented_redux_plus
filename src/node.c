#include "node.h"

#define INDENT_INCREMENT 2

static void node_fprintln_indent(FILE* file, const Node* node, size_t indent);
static void type_node_fprintln_indent(FILE* file, const TypeNode* type_node, size_t indent);

static void process_indent(FILE* file, size_t* indent) {
    for (size_t i = 0; i < *indent; i++) {
        fputc('-', file);
    }

    *indent = *indent + INDENT_INCREMENT;
}

static void expr_node_fprintln_indent(FILE* file, const ExprNode* expr_node, size_t indent) {
    process_indent(file, &indent);

    switch (expr_node->type) {
        case EXPR_NODE_INT: {
            fprintf(file, "Int("SV_FMT")\n", SV_ARG(expr_node->as.literal.token.text));
        } break;
        case EXPR_NODE_FLOAT: {
            fprintf(file, "Float("SV_FMT")\n", SV_ARG(expr_node->as.literal.token.text));
        } break;
        case EXPR_NODE_BOOL: {
            fprintf(file, "Bool("SV_FMT")\n", SV_ARG(expr_node->as.literal.token.text));
        } break;
        case EXPR_NODE_CHAR: {
            fprintf(file, "Char("SV_FMT")\n", SV_ARG(expr_node->as.literal.token.text));
        } break;
        case EXPR_NODE_VAR: {
            fprintf(file, "Var("SV_FMT")\n", SV_ARG(expr_node->as.var.token.text));
        } break;
        case EXPR_NODE_UNARY_OP: {
            fprintf(file, "UnaryOp("SV_FMT"):\n", SV_ARG(expr_node->as.unary_op.token.text));
            expr_node_fprintln_indent(file, expr_node->as.unary_op.operand, indent);
        } break;
        case EXPR_NODE_BIN_OP: {
            fprintf(file, "BinOp("SV_FMT"):\n", SV_ARG(expr_node->as.bin_op.token.text));
            expr_node_fprintln_indent(file, expr_node->as.bin_op.left, indent);
            expr_node_fprintln_indent(file, expr_node->as.bin_op.right, indent);
        } break;
        case EXPR_NODE_LOGICAL_OP: {
            fprintf(file, "BinOp("SV_FMT"):\n", SV_ARG(expr_node->as.logical_op.token.text));
            expr_node_fprintln_indent(file, expr_node->as.logical_op.left, indent);
            expr_node_fprintln_indent(file, expr_node->as.logical_op.right, indent);
        } break;
        case EXPR_NODE_CAST: {
            fprintf(file, "Cast:\n");
            expr_node_fprintln_indent(file, expr_node->as.cast.expr, indent);
            type_node_fprintln_indent(file, (TypeNode*) expr_node->as.cast.type, indent);
        } break;
        case EXPR_NODE_ASSIGNMENT: {
            fprintf(file, "Assignment:\n");
            expr_node_fprintln_indent(file, expr_node->as.assignment.left, indent);
            expr_node_fprintln_indent(file, expr_node->as.assignment.right, indent);
        } break;
        case EXPR_NODE_BLOCK: {
            fprintf(file, "Block:\n");

            for (size_t i = 0; i < expr_node->as.block.nodes.length; i++) {
                node_fprintln_indent(file, expr_node->as.block.nodes.data[i], indent);
            }
        } break;
        case EXPR_NODE_IF_ELSE: {
            fprintf(file, "If:\n");
            expr_node_fprintln_indent(file, expr_node->as.if_else.condition, indent);
            expr_node_fprintln_indent(file, (ExprNode*) expr_node->as.if_else.if_branch, indent);

            if (expr_node->as.if_else.else_branch) {
                expr_node_fprintln_indent(file, expr_node->as.if_else.else_branch, indent);
            }
        } break;
    }
}

static void decl_node_fprintln_indent(FILE* file, const DeclNode* decl_node, size_t indent) {
    process_indent(file, &indent);

    switch (decl_node->type) {
        case DECL_NODE_VAR: {
            fprintf(file, "Let("SV_FMT"):\n", SV_ARG(decl_node->as.var_decl.var_name.text));
            if (decl_node->as.var_decl.type) type_node_fprintln_indent(file, decl_node->as.var_decl.type, indent);
            expr_node_fprintln_indent(file, decl_node->as.var_decl.expr, indent);
        } break;
    }
}

static void type_node_fprintln_indent(FILE* file, const TypeNode* type_node, size_t indent) {
    process_indent(file, &indent);

    switch (type_node->type) {
        case TYPE_NODE_PRIMITIVE: {
            fprintf(file, "PrimitiveType(%s)\n", primitive_type_to_string(type_node->as.primitive_type.type));
        } break;
        case TYPE_NODE_MONOSTATE: {
            fprintf(file, "MonostateType\n");
        } break;
    }
}

static void node_fprintln_indent(FILE* file, const Node* node, size_t indent) {
    switch (node->type) {
        case NODE_EXPR: {
            expr_node_fprintln_indent(file, &node->as.expr, indent);
        } break;
        case NODE_DECL: {
            decl_node_fprintln_indent(file, &node->as.decl, indent);
        } break;
        case NODE_TYPE: {
            type_node_fprintln_indent(file, &node->as.type, indent);
        } break;
    }
}

void expr_node_fprintln(FILE* file, const ExprNode* expr_node) {
    expr_node_fprintln_indent(file, expr_node, 0);
}

void decl_node_fprintln(FILE* file, const DeclNode* decl_node) {
    decl_node_fprintln_indent(file, decl_node, 0);
}

void type_node_fprintln(FILE* file, const TypeNode* type_node) {
    type_node_fprintln_indent(file, type_node, 0);
}

void node_fprintln(FILE* file, const Node* node) {
    node_fprintln_indent(file, node, 0);
}

void program_fprintln(FILE* file, Program program) {
    fprintf(file, "Program:\n");
    for (size_t i = 0; i < program.length; i++) {
        node_fprintln_indent(file, program.data[i], INDENT_INCREMENT);
    }
}
