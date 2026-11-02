#pragma once

#include <stddef.h>
#include <stdbool.h>

#include "string_view.h"
#include "token.h"

typedef struct {
    StringView src;
    size_t i;
    size_t line;
    size_t pos;
    size_t start_i;
    size_t start_line;
    size_t start_pos;
} Lexer;

void lexer_init(Lexer* lexer, StringView src);

bool lexer_next(Lexer* lexer, Token* out);

void lexer_skip_int(Lexer* lexer);

void lexer_skip_whitespace(Lexer* lexer);

void lexer_setup_start(Lexer* lexer);

char lexer_peek(Lexer* lexer);

char lexer_advance(Lexer* lexer);

bool lexer_matches(Lexer* lexer, char c);
