#pragma once

#include <stdbool.h>

#include "token.h"
#include "lexer.h"
#include "node.h"

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
    Sega_Node* sega;
} Parser;

bool parser_init(Parser* parser, StringView src);

Node* parser_new_node(Parser* parser, Node node);

Node* parser_parse(Parser* parser, Sega_Node* sega);

Node* parser_expr(Parser* parser);

Node* parser_prec(Parser* parser, Precedence prev_prec);

Node* parser_prefix_expr(Parser* parser);

bool parser_advance(Parser* parser);

bool parser_expect(Parser* parser, TokenType tt);
