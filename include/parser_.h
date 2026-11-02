#pragma once

#include <stdbool.h>

#include "token.h"
#include "lexer.h"

typedef enum {
    PREC_NONE,
    PREC_ADD,
    PREC_MUL,
} Precedence;

Precedence prec_from(TokenType tt);

typedef struct {
    Lexer lexer;
    Token cur;
    Token lookahead;
} Parser;

bool parser_init(Parser* parser, StringView src);

bool parser_parse(Parser* parser, int* out);

bool parser_expr(Parser* parser, int* out);

bool parser_prec(Parser* parser, int* out, Precedence prev_prec);

bool parser_prefix_expr(Parser* parser, int* out);

bool parser_advance(Parser* parser);

bool parser_expect(Parser* parser, TokenType tt);
