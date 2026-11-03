#include "parser_.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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

bool parser_init(Parser* self, StringView src) {
    lexer_init(&self->lexer, src);
    return lexer_next(&self->lexer, &self->cur) && lexer_next(&self->lexer, &self->lookahead);
}

Node* parser_new_node(Parser* self, Node node) {
    sega_append(self->sega, node);
    return sega_get_ptr(self->sega, sega_length(*self->sega) - 1);
}

Node* parser_parse(Parser* self, Sega_Node* sega) {
    self->sega = sega;
    Node* node = parse_expr(self);
    return node && parser_expect(self, TT_EOF) ? node : NULL;
}

Node* parse_expr(Parser* self) {
    return parse_prec(self, PREC_NONE);
}

Node* parse_prec(Parser* self, Precedence prev_prec) {
    Node* lhs = parse_prefix_expr(self);
    if (!lhs) return NULL;

    Precedence next_prec;
    while ((next_prec = prec_from(self->cur.type)) > prev_prec) {
        Token op = self->cur;
        parser_advance(self);

        Node* rhs = parse_prec(self, next_prec);
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
            case TT_INT:
            case TT_EOF: {
                fprintf(stderr, "Invalid binary operator '"SV_FMT"'. This should never happen.\n", SV_ARG(op.text));
                exit(-1);
            } break;
        }

        lhs = parser_new_node(self, (Node) {
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

Node* parse_prefix_expr(Parser* self) {
    switch (self->cur.type) {
        case TT_INT: {
            Token token = self->cur;
            parser_advance(self);

            return parser_new_node(self, (Node) {
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
            parser_error_expected_cstr(self, "expression");
            return NULL;
        } break;
    }

    return NULL;
}

bool parser_advance(Parser* self) {
    self->cur = self->lookahead;
    return lexer_next(&self->lexer, &self->lookahead);
}

bool parser_matches(Parser* self, TokenType tt) {
    return self->cur.type == tt && parser_advance(self);
}

bool parser_expect(Parser* self, TokenType tt) {
    if (!parser_matches(self, tt)) {
        parser_error_expected_cstr(self, tt_str(tt));
        return false;
    }

    return true;
}

void parser_error(Parser* self, const char* fmt, ...) {
    (void) self;

    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Syntax error at line %zu: ", self->cur.line_number + 1);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ".\n");
    fprintf(stderr, SV_FMT"\n", SV_ARG(self->cur.line_text));

    for (size_t i = 0; i < self->cur.start_col; i++) {
        fputc(' ', stderr);
    }

    for (size_t i = self->cur.start_col; i < self->cur.end_col; i++) {
        fputc('^', stderr);
    }

    fputc('\n', stderr);

    va_end(args);
}

void parser_error_expected(Parser* self, StringView expected) {
    parser_error(self, "expected "SV_FMT", but got '"SV_FMT"'", SV_ARG(expected), SV_ARG(self->cur.text));
}

void parser_error_expected_cstr(Parser* self, const char* expected) {
    parser_error(self, "expected %s, but got '"SV_FMT"'", expected, SV_ARG(self->cur.text));
}
