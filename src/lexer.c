#include "lexer.h"

#include "char_utils.h"

void lexer_init(Lexer* lexer, StringView src) {
    *lexer = (Lexer) {
        .src = src,
        .i = 0,
        .line = 1,
        .pos = 1,
    };
}

bool lexer_next(Lexer* lexer, Token* out) {
    lexer_skip_whitespace(lexer);
    lexer_setup_start(lexer);

    char c = lexer_advance(lexer);
    TokenType tt;

    switch (c) {
        case '+': {
            tt = TT_PLUS;
        } break;
        case '-': {
            tt = TT_MINUS;
        } break;
        case '*': {
            tt = TT_TIMES;
        } break;
        case '/': {
            tt = TT_DIV;
        } break;
        case '\0': {
            tt = TT_EOF;
        } break;
        default: {
            if (is_ascii_digit(c)) {
                tt = TT_INT;
                lexer_skip_int(lexer);
            } else {
                return false;
            }
        } break;
    }

    *out = (Token) {
        .type = tt,
        .text = tt == TT_EOF ? sv_from_literal("") : sv_slice(lexer->src, lexer->start_i, lexer->i - lexer->start_i),
    };

    return true;
}

void lexer_skip_int(Lexer* lexer) {
    while (is_ascii_digit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
}

void lexer_skip_whitespace(Lexer* lexer) {
    while (true) {
        char c = lexer_peek(lexer);

        if (is_ascii_whitespace(c)) {
            lexer_advance(lexer);
            continue;
        }

        if (c == '#') {
            while (lexer_advance(lexer) != '\n');
        }

        break;
    }
}

void lexer_setup_start(Lexer* lexer) {
    lexer->start_i = lexer->i;
    lexer->start_line = lexer->line;
    lexer->start_pos = lexer->pos;
}

char lexer_peek(Lexer* lexer) {
    return lexer->i < lexer->src.length ? lexer->src.data[lexer->i] : '\0';
}

char lexer_advance(Lexer* lexer) {
    char c = lexer_peek(lexer);
    lexer->i++;
    return c;
}

bool lexer_matches(Lexer* lexer, char c) {
    if (lexer_peek(lexer) == c) {
        lexer_advance(lexer);
        return true;
    }

    return false;
}
