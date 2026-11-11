#pragma once

#include <stdio.h>

#include "string_view.h"
#include "dynamic_array.h"

typedef enum {
    TT_INT,
    TT_FLOAT,
    TT_CHAR,

    TT_PLUS,
    TT_MINUS,
    TT_TIMES,
    TT_DIV,
    TT_MOD,

    TT_AND,
    TT_XOR,
    TT_OR,
    TT_LTLT,
    TT_GTGT,
    TT_TILDE,

    TT_ANDAND,
    TT_OROR,
    TT_BANG,

    TT_EQEQ,
    TT_BANG_EQ,
    TT_LT,
    TT_LTEQ,
    TT_GT,
    TT_GTEQ,

    TT_EQ,

    TT_LPAREN,
    TT_RPAREN,

    TT_LBRACE,
    TT_RBRACE,

    TT_COLON,
    TT_SEMI,

    TT_ID,

    TT_KW_LET,
    TT_KW_MUT,

    TT_KW_IF,
    TT_KW_ELSE,

    TT_KW_AS,

    TT_KW_INT,
    TT_KW_FLOAT,
    TT_KW_BOOL,
    TT_KW_TRUE,
    TT_KW_FALSE,
    TT_KW_CHAR,
    TT_KW_MONOSTATE,

    TT_EOF,
} TokenType;

const char* tt_repr(TokenType tt);

const char* tt_error_message_name(TokenType tt);

bool tt_is_keyword(TokenType tt);

typedef struct {
    TokenType type;
    StringView text;
    StringView line_text;
    size_t line_number;
    size_t start_col;
    size_t end_col;
} Token;

void token_show_in_line(FILE* file, Token token);

typedef DynamicArray(Token) Tokens;
