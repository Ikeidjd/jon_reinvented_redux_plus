#pragma once

#include "string_view.h"
#include "dynamic_array.h"

typedef enum {
    TT_INT,
    TT_PLUS,
    TT_MINUS,
    TT_TIMES,
    TT_DIV,
    TT_EOF,
} TokenType;

const char* tt_repr(TokenType tt);

const char* tt_str(TokenType tt);

typedef struct {
    TokenType type;
    StringView text;
    StringView line_text;
    size_t line_number;
    size_t start_col;
    size_t end_col;
} Token;

typedef DynamicArray(Token) Tokens;
