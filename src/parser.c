#include "parser_.h"

#include <stdio.h>
#include <stdlib.h>

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

bool parser_parse(Parser* parser, int* out) {
    return parser_expr(parser, out) && parser_expect(parser, TT_EOF);
}

bool parser_expr(Parser* parser, int* out) {
    return parser_prec(parser, out, PREC_NONE);
}

bool parser_prec(Parser* parser, int* out, Precedence prev_prec) {
    if (!parser_prefix_expr(parser, out)) return false;

    Precedence next_prec;
    while ((next_prec = prec_from(parser->cur.type)) > prev_prec) {
        Token op = parser->cur;
        parser_advance(parser);

        int aux;
        if (!parser_prec(parser, &aux, next_prec)) return false;

        switch (op.type) {
            case TT_PLUS: {
                *out = *out + aux;
            } break;
            case TT_MINUS: {
                *out = *out - aux;
            } break;
            case TT_TIMES: {
                *out = *out * aux;
            } break;
            case TT_DIV: {
                *out = *out / aux;
            } break;
            case TT_INT:
            case TT_EOF: {
                fprintf(stderr, "Invalid binary operator '"SV_FMT"'. This should never happen.\n", SV_ARG(op.text));
                exit(-1);
            } break;
        }
    }

    return true;
}

bool parser_prefix_expr(Parser* parser, int* out) {
    switch (parser->cur.type) {
        case TT_INT: {
            if (!sv_to_int(parser->cur.text, out)) return false;
            parser_advance(parser);
            return true;
        } break;
        case TT_PLUS:
        case TT_MINUS:
        case TT_TIMES:
        case TT_DIV:
        case TT_EOF: {
            return false;
        } break;
    }

    return false;
}

bool parser_advance(Parser* parser) {
    parser->cur = parser->lookahead;
    return lexer_next(&parser->lexer, &parser->lookahead);
}

bool parser_expect(Parser* parser, TokenType tt) {
    return parser->cur.type == tt && parser_advance(parser);
}
