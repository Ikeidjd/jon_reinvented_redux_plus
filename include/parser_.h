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

bool parser_init(Parser* self, StringView src);

Node* parser_new_node(Parser* self, Node node);

Node* parser_parse(Parser* self, Sega_Node* sega);

Node* parse_expr(Parser* self);

Node* parse_prec(Parser* self, Precedence prev_prec);

Node* parse_prefix_expr(Parser* self);

bool parser_advance(Parser* self);

bool parser_matches(Parser* self, TokenType tt);

bool parser_expect(Parser* self, TokenType tt);

void parser_error(Parser* self, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

void parser_error_expected(Parser* self, StringView expected);

void parser_error_expected_cstr(Parser* self, const char* expected);
