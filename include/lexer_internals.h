#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "string_view.h"
#include "token.h"

typedef struct {
    StringView src;
    StringView line_text;
    size_t i;
    size_t line;
    size_t col;
    size_t start_i;
    size_t start_line;
    size_t start_col;
} Lexer;

void lexer_init(Lexer* self, StringView src);

bool lexer_next(Lexer* self, Token* out);

void lexer_skip_int(Lexer* self);

TokenType lexer_skip_identifier(Lexer* self);

void lexer_skip_whitespace(Lexer* self);

void lexer_setup_start(Lexer* self);

char lexer_peek(Lexer* self);

char lexer_advance(Lexer* self);

bool lexer_matches(Lexer* self, char c);
