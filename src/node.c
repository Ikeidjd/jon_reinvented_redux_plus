#include "node.h"

#define INDENT_INCREMENT 2

static void node_fprintln_indent(FILE* file, const Node* node, size_t indent);
static void type_node_fprintln_indent(FILE* file, const TypeNode* type, size_t indent);

static void process_indent(FILE* file, size_t* indent) {
    for (size_t i = 0; i < *indent; i++) {
        fputc('-', file);
    }

    *indent = *indent + INDENT_INCREMENT;
}

static void expr_node_fprintln_indent(FILE* file, const ExprNode* expr, size_t indent) {
    process_indent(file, &indent);

    switch (expr->type) {
        case EXPR_NODE_INT: {
            fprintf(file, "Int("SV_FMT")\n", SV_ARG(expr->as.literal.token.text));
        } break;
        case EXPR_NODE_FLOAT: {
            fprintf(file, "Float("SV_FMT")\n", SV_ARG(expr->as.literal.token.text));
        } break;
        case EXPR_NODE_BOOL: {
            fprintf(file, "Bool("SV_FMT")\n", SV_ARG(expr->as.literal.token.text));
        } break;
        case EXPR_NODE_CHAR: {
            fprintf(file, "Char("SV_FMT")\n", SV_ARG(expr->as.literal.token.text));
        } break;
        case EXPR_NODE_VAR: {
            fprintf(file, "Var("SV_FMT")\n", SV_ARG(expr->as.var.token.text));
        } break;
        case EXPR_NODE_UNARY_OP: {
            fprintf(file, "UnaryOp("SV_FMT"):\n", SV_ARG(expr->as.unary_op.token.text));
            expr_node_fprintln_indent(file, expr->as.unary_op.operand, indent);
        } break;
        case EXPR_NODE_BIN_OP: {
            fprintf(file, "BinOp("SV_FMT"):\n", SV_ARG(expr->as.bin_op.token.text));
            expr_node_fprintln_indent(file, expr->as.bin_op.left, indent);
            expr_node_fprintln_indent(file, expr->as.bin_op.right, indent);
        } break;
        case EXPR_NODE_LOGICAL_OP: {
            fprintf(file, "BinOp("SV_FMT"):\n", SV_ARG(expr->as.logical_op.token.text));
            expr_node_fprintln_indent(file, expr->as.logical_op.left, indent);
            expr_node_fprintln_indent(file, expr->as.logical_op.right, indent);
        } break;
        case EXPR_NODE_CAST: {
            fprintf(file, "Cast:\n");
            expr_node_fprintln_indent(file, expr->as.cast.expr, indent);
            type_node_fprintln_indent(file, (TypeNode*) expr->as.cast.type, indent);
        } break;
        case EXPR_NODE_ASSIGNMENT: {
            fprintf(file, "Assignment:\n");
            expr_node_fprintln_indent(file, expr->as.assignment.left, indent);
            expr_node_fprintln_indent(file, expr->as.assignment.right, indent);
        } break;
        case EXPR_NODE_BLOCK: {
            fprintf(file, "Block:\n");

            for (size_t i = 0; i < expr->as.block.nodes.length; i++) {
                node_fprintln_indent(file, expr->as.block.nodes.data[i], indent);
            }
        } break;
        case EXPR_NODE_IF_ELSE: {
            fprintf(file, "If:\n");
            expr_node_fprintln_indent(file, expr->as.if_else.condition, indent);
            expr_node_fprintln_indent(file, (ExprNode*) expr->as.if_else.if_branch, indent);

            if (expr->as.if_else.else_branch) {
                expr_node_fprintln_indent(file, expr->as.if_else.else_branch, indent);
            }
        } break;
    }
}

static void stat_node_fprintln_indent(FILE* file, const StatNode* stat, size_t indent) {
    process_indent(file, &indent);

    switch (stat->type) {
        case STAT_NODE_WHILE_LOOP: {
            fprintf(file, "While:\n");
            expr_node_fprintln_indent(file, stat->as.while_loop.condition, indent);
            expr_node_fprintln_indent(file, (ExprNode*) stat->as.while_loop.body, indent);
        } break;
        case STAT_NODE_PRINT: {
            fprintf(file, "Print:\n");
            expr_node_fprintln_indent(file, stat->as.print.expr, indent);
        } break;
    }
}

static void decl_node_fprintln_indent(FILE* file, const DeclNode* decl, size_t indent) {
    process_indent(file, &indent);

    switch (decl->type) {
        case DECL_NODE_VAR: {
            fprintf(file, "Let("SV_FMT"):\n", SV_ARG(decl->as.var_decl.var_name.text));
            if (decl->as.var_decl.type) type_node_fprintln_indent(file, decl->as.var_decl.type, indent);
            expr_node_fprintln_indent(file, decl->as.var_decl.expr, indent);
        } break;
    }
}

static void type_node_fprintln_indent(FILE* file, const TypeNode* type, size_t indent) {
    process_indent(file, &indent);

    switch (type->type) {
        case TYPE_NODE_PRIMITIVE: {
            fprintf(file, "PrimitiveType(%s)\n", primitive_type_to_string(type->as.primitive_type.type));
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
        case NODE_STAT: {
            stat_node_fprintln_indent(file, &node->as.stat, indent);
        } break;
        case NODE_DECL: {
            decl_node_fprintln_indent(file, &node->as.decl, indent);
        } break;
        case NODE_TYPE: {
            type_node_fprintln_indent(file, &node->as.type, indent);
        } break;
    }
}

void expr_node_fprintln(FILE* file, const ExprNode* expr) {
    expr_node_fprintln_indent(file, expr, 0);
}

void stat_node_fprintln(FILE* file, const StatNode* stat) {
    stat_node_fprintln_indent(file, stat, 0);
}

void decl_node_fprintln(FILE* file, const DeclNode* decl) {
    decl_node_fprintln_indent(file, decl, 0);
}

void type_node_fprintln(FILE* file, const TypeNode* type) {
    type_node_fprintln_indent(file, type, 0);
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
