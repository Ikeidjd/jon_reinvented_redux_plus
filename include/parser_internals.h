#pragma once

#include <stdbool.h>

#include "token.h"
#include "lexer.h"
#include "node.h"

typedef enum {
    PREC_NONE,
    PREC_ASSIGN,
    PREC_COMPARISON,
    PREC_BIT_OR,
    PREC_BIT_XOR,
    PREC_BIT_AND,
    PREC_BIT_SHIFT,
    PREC_ADD,
    PREC_MUL,
    PREC_UNARY_PREFIX,
} Precedence;

// Hehe... ass
typedef enum {
    ASS_LEFT,
    ASS_RIGHT,
    ASS_PAREN,
} Associativity;

static Precedence prec_from(TokenType tt);

static Associativity prec_get_associativity(Precedence prec);

typedef struct {
    Sega_Node node_buffer;
    Tokens tokens;
    size_t i;
} Parser;

static void parser_init(Parser* self, Tokens tokens, Sega_Node node_buffer);

static ExprNode* parser_new_expr(Parser* self, ExprNode node);

static DeclNode* parser_new_decl(Parser* self, DeclNode decl);

static TypeNode* parser_new_type(Parser* self, TypeNode type);

static bool parser_parse(Parser* self, Program* program);

static TypeNode* parse_type(Parser* self);

static DeclNode* parse_decl(Parser* self);

static BinOpType tt_to_bin_op_type(TokenType tt);

static UnaryOpType tt_to_unary_op_type(TokenType tt);

static ExprNode* parse_expr(Parser* self);

static ExprNode* parse_prec(Parser* self, Precedence prev_prec);

static ExprNode* parse_prefix_expr(Parser* self);

static Token parser_cur(Parser* self);

static void parser_advance(Parser* self);

static bool parser_has(Parser* self, TokenType tt);

static bool parser_matches(Parser* self, TokenType tt);

static bool parser_expect(Parser* self, TokenType tt);

static void parser_error(Parser* self, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

static void parser_error_expected(Parser* self, StringView expected);

static void parser_error_expected_cstr(Parser* self, const char* expected);
