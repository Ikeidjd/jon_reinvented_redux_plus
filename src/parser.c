#include "parser_.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

Precedence prec_from(TokenType tt) {
    switch (tt) {
        case TT_PLUS:
        case TT_MINUS:
            return PREC_ADD;
        case TT_TIMES:
        case TT_DIV:
            return PREC_MUL;
        case TT_INT:
        case TT_EOF:
            return PREC_NONE;
    }

    return PREC_NONE;
}

bool parser_init(Parser* parser, StringView src) {
    lexer_init(&parser->lexer, src);
    return lexer_next(&parser->lexer, &parser->cur) && lexer_next(&parser->lexer, &parser->lookahead);
}

Node* parser_new_node(Parser* parser, Node node) {
    sega_append(parser->sega, node);
    return sega_get_ptr(parser->sega, sega_length(*parser->sega) - 1);
}

Node* parser_parse(Parser* parser, Sega_Node* sega) {
    parser->sega = sega;
    Node* node = parser_expr(parser);
    if (!parser_expect(parser, TT_EOF)) return NULL;
    return node;
}

Node* parser_expr(Parser* parser) {
    return parser_prec(parser, PREC_NONE);
}

Node* parser_prec(Parser* parser, Precedence prev_prec) {
    Node* lhs = parser_prefix_expr(parser);
    if (!lhs) return false;

    Precedence next_prec;
    while ((next_prec = prec_from(parser->cur.type)) > prev_prec) {
        Token op = parser->cur;
        parser_advance(parser);

        Node* rhs = parser_prec(parser, next_prec);
        if (!rhs) return false;

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
            case TT_INT:
            case TT_EOF: {
                fprintf(stderr, "Invalid binary operator '"SV_FMT"'. This should never happen.\n", SV_ARG(op.text));
                exit(-1);
            } break;
        }

        lhs = parser_new_node(parser, (Node) {
            .type = NODE_BIN_OP,
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

Node* parser_prefix_expr(Parser* parser) {
    switch (parser->cur.type) {
        case TT_INT: {
            Token token = parser->cur;
            parser_advance(parser);

            return parser_new_node(parser, (Node) {
                .type = NODE_INT,
                .as.integer = {
                    .token = token,
                },
            });
        } break;
        case TT_PLUS:
        case TT_MINUS:
        case TT_TIMES:
        case TT_DIV:
        case TT_EOF: {
            return NULL;
        } break;
    }

    return NULL;
}

bool parser_advance(Parser* parser) {
    parser->cur = parser->lookahead;
    return lexer_next(&parser->lexer, &parser->lookahead);
}

bool parser_expect(Parser* parser, TokenType tt) {
    return parser->cur.type == tt && parser_advance(parser);
}
