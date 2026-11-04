#pragma once

#include <stdbool.h>

#include "dynamic_array.h"
#include "token.h"
#include "lexer.h"
#include "node.h"

typedef enum {
    PREC_NONE,
    PREC_ASSIGN,
    PREC_ADD,
    PREC_MUL,
} Precedence;

Precedence prec_from(TokenType tt);

bool prec_is_left_associative(Precedence prec);

typedef struct {
    Sega_Node node_buffer;
    Tokens tokens;
    size_t i;
} Parser;

void parser_init(Parser* self, Tokens tokens, Sega_Node node_buffer);

NodeExpr* parser_new_expr(Parser* self, NodeExpr node);

bool parser_parse(Parser* self, Program* program);

NodeDecl* parse_decl(Parser* self);

NodeExpr* parse_expr(Parser* self);

NodeExpr* parse_prec(Parser* self, Precedence prev_prec);

NodeExpr* parse_prefix_expr(Parser* self);

Token parser_cur(Parser* self);

void parser_advance(Parser* self);

bool parser_has(Parser* self, TokenType tt);

bool parser_matches(Parser* self, TokenType tt);

bool parser_expect(Parser* self, TokenType tt);

void parser_error(Parser* self, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

void parser_error_expected(Parser* self, StringView expected);

void parser_error_expected_cstr(Parser* self, const char* expected);
