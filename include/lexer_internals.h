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

static void lexer_init(Lexer* self, StringView src);

static bool lexer_next(Lexer* self, Token* out);

static bool lexer_skip_number(Lexer* self, TokenType* tt, char c);

static bool lexer_skip_char(Lexer* self);

static TokenType lexer_skip_identifier(Lexer* self);

static void lexer_skip_whitespace(Lexer* self);

static void lexer_setup_start(Lexer* self);

static Token lexer_get_cur_token(Lexer* self, TokenType tt);

static char lexer_peek(Lexer* self);

static char lexer_advance(Lexer* self);

static bool lexer_matches(Lexer* self, char c);

static void lexer_error(Lexer* self, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
