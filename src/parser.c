#include "parser_.h"
#include "parser_internals.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

bool parse_program(Tokens tokens, Sega_Node* node_buffer, Program* program) {
    Parser self;
    parser_init(&self, tokens, *node_buffer);

    bool b = parser_parse(&self, program);
    *node_buffer = self.node_buffer;

    return b;
}

Precedence prec_from(TokenType tt) {
    switch (tt) {
        case TT_EQ:
            return PREC_ASSIGN;
        case TT_PLUS:
        case TT_MINUS:
            return PREC_ADD;
        case TT_TIMES:
        case TT_DIV:
        case TT_MOD:
            return PREC_MUL;
        default:
            return PREC_NONE;
    }
}

bool prec_is_left_associative(Precedence prec) {
    switch (prec) {
        case PREC_NONE:
        case PREC_ADD:
        case PREC_MUL:
            return true;
        case PREC_ASSIGN:
            return false;
    }

    return true;
}

void parser_init(Parser* self, Tokens tokens, Sega_Node node_buffer) {
    *self = (Parser) {
        .node_buffer = node_buffer,
        .tokens = tokens,
        .i = 0,
    };
}

NodeExpr* parser_new_expr(Parser* self, NodeExpr expr) {
    Node node = {
        .type = NODE_EXPR,
        .as.expr = expr,
    };

    sega_append(&self->node_buffer, node);
    return &sega_get_ptr(&self->node_buffer, sega_length(self->node_buffer) - 1)->as.expr;
}

NodeDecl* parser_new_decl(Parser* self, NodeDecl decl) {
    Node node = {
        .type = NODE_DECL,
        .as.decl = decl,
    };

    sega_append(&self->node_buffer, node);
    return &sega_get_ptr(&self->node_buffer, sega_length(self->node_buffer) - 1)->as.decl;
}

bool parser_parse(Parser* self, Program* program) {
    bool success = true;

    while (!parser_matches(self, TT_EOF)) {
        Node* node;
        switch (parser_cur(self).type) {
            case TT_KW_LET: {
                node = (Node*) parse_decl(self);
            } break;
            default: {
                node = (Node*) parse_expr(self);
                if (node && !parser_expect(self, TT_SEMI)) node = NULL;
            } break;
        }

        if (node) {
            dyna_append(program, node);
        } else {
            success = false;

            // Giving multiple errors on one statement would be a bit of a mess
            while (!parser_matches(self, TT_SEMI) && !parser_matches(self, TT_EOF)) {
                parser_advance(self);
            }
        }
    }

    return success;
}

NodeDecl* parse_decl(Parser* self) {
    if (!parser_expect(self, TT_KW_LET)) return NULL;

    Token var_name = parser_cur(self);
    if (!parser_expect(self, TT_ID)) return NULL;

    Token equals_sign = parser_cur(self);
    if (!parser_expect(self, TT_EQ)) return NULL;

    NodeExpr* expr = parse_expr(self);
    if (!expr) return NULL;

    if (!parser_expect(self, TT_SEMI)) return NULL;

    return parser_new_decl(self, (NodeDecl) {
        .type = DECL_VAR,
        .as.var_decl = {
            .var_name = var_name,
            .equals_sign = equals_sign,
            .expr = expr,
        },
    });
}

NodeExpr* parse_expr(Parser* self) {
    return parse_prec(self, PREC_NONE);
}

NodeExpr* parse_prec(Parser* self, Precedence prev_prec) {
    if (prec_is_left_associative(prev_prec)) prev_prec++;

    NodeExpr* lhs = parse_prefix_expr(self);
    if (!lhs) return NULL;

    Precedence next_prec;
    while ((next_prec = prec_from(parser_cur(self).type)) >= prev_prec) {
        Token op = parser_cur(self);
        parser_advance(self);

        NodeExpr* rhs = parse_prec(self, next_prec);
        if (!rhs) return NULL;

        BinOpType op_type;
        switch (op.type) {
            case TT_PLUS: {
                op_type = BIN_ADD;
            } break;
            case TT_MINUS: {
                op_type = BIN_SUB;
            } break;
            case TT_TIMES: {
                op_type = BIN_MUL;
            } break;
            case TT_DIV: {
                op_type = BIN_DIV;
            } break;
            case TT_MOD: {
                op_type = BIN_MOD;
            } break;
            case TT_EQ: {
                op_type = BIN_ASSIGN;
            } break;
            default: {
                fprintf(stderr, "Invalid binary operator '"SV_FMT"'. This should never happen.\n", SV_ARG(op.text));
                exit(-1);
            } break;
        }

        lhs = parser_new_expr(self, (NodeExpr) {
            .type = EXPR_BIN_OP,
            .as.bin_op = {
                .type = op_type,
                .token = op,
                .left = lhs,
                .right = rhs,
            },
        });
    }

    return lhs;
}

NodeExpr* parse_prefix_expr(Parser* self) {
    switch (parser_cur(self).type) {
        case TT_INT: {
            Token token = parser_cur(self);
            parser_advance(self);

            return parser_new_expr(self, (NodeExpr) {
                .type = EXPR_INT,
                .as.integer = {
                    .token = token,
                },
            });
        } break;
        case TT_ID: {
            Token token = parser_cur(self);
            parser_advance(self);

            return parser_new_expr(self, (NodeExpr) {
                .type = EXPR_VAR,
                .as.var = {
                    .token = token,
                },
            });
        } break;
        default: {
            parser_error_expected_cstr(self, "expression");
            return NULL;
        } break;
    }
}

Token parser_cur(Parser* self) {
    return self->tokens.data[self->i];
}

void parser_advance(Parser* self) {
    if (parser_cur(self).type != TT_EOF) self->i++;
}

bool parser_has(Parser* self, TokenType tt) {
    return parser_cur(self).type == tt;
}

bool parser_matches(Parser* self, TokenType tt) {
    if (parser_has(self, tt)) {
        parser_advance(self);
        return true;
    }

    return false;
}

bool parser_expect(Parser* self, TokenType tt) {
    if (!parser_matches(self, tt)) {
        parser_error_expected_cstr(self, tt_error_message_name(tt));
        return false;
    }

    return true;
}

void parser_error(Parser* self, const char* fmt, ...) {
    (void) self;

    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Syntax error at line %zu: ", parser_cur(self).line_number + 1);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ".\n");
    fprintf(stderr, SV_FMT"\n", SV_ARG(parser_cur(self).line_text));

    for (size_t i = 0; i < parser_cur(self).start_col; i++) {
        fputc(' ', stderr);
    }

    for (size_t i = parser_cur(self).start_col; i < parser_cur(self).end_col; i++) {
        fputc('^', stderr);
    }

    fputc('\n', stderr);

    va_end(args);
}

void parser_error_expected(Parser* self, StringView expected) {
    parser_error(self, "expected "SV_FMT", but got '"SV_FMT"'", SV_ARG(expected), SV_ARG(parser_cur(self).text));
}

void parser_error_expected_cstr(Parser* self, const char* expected) {
    parser_error(self, "expected %s, but got '"SV_FMT"'", expected, SV_ARG(parser_cur(self).text));
}
