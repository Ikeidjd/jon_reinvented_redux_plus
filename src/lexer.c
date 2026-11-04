#include "lexer.h"
#include "lexer_internals.h"

#include <stdio.h>

#include "char_utils.h"

typedef struct {
    const char* name;
    TokenType tt;
} Keyword;

static Keyword keywords[] = {
    { "let", TT_KW_LET },
};

bool lex_program(StringView src, Tokens* token_buf) {
    Lexer self;
    lexer_init(&self, src);

    Token token;
    while (lexer_next(&self, &token)) {
        dyna_append(token_buf, token);
        if (token.type == TT_EOF) return true;
    }

    return false;
}

void lexer_init(Lexer* self, StringView src) {
    *self = (Lexer) {
        .src = src,
        .line_text = sv_first_line(src),
        .i = 0,
        .line = 0,
        .col = 0,
    };
}

bool lexer_next(Lexer* self, Token* out) {
    lexer_skip_whitespace(self);
    lexer_setup_start(self);

    char c = lexer_advance(self);
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
        case '%': {
            tt = TT_MOD;
        } break;
        case '=': {
            tt = TT_EQ;
        } break;
        case ':': {
            tt = TT_COLON;
        } break;
        case ';': {
            tt = TT_SEMI;
        } break;
        case '\0': {
            tt = TT_EOF;
        } break;
        default: {
            if (is_ascii_digit(c)) {
                tt = TT_INT;
                lexer_skip_int(self);
            } else if (is_ascii_alpha(c) || c == '_') {
                tt = lexer_skip_identifier(self);
            } else {
                fprintf(stderr, "Syntax error at line %zu, pos %zu: invalid character '%c'.\n", self->start_line, self->start_col, c);
                return false;
            }
        } break;
    }

    *out = (Token) {
        .type = tt,
        .text = tt == TT_EOF ? sv_lit("") : sv_slice(self->src, self->start_i, self->i - self->start_i),
        .line_text = self->line_text,
        .line_number = self->line,
        .start_col = self->start_col,
        .end_col = self->col,
    };

    return true;
}

void lexer_skip_int(Lexer* self) {
    while (is_ascii_digit(lexer_peek(self))) {
        lexer_advance(self);
    }
}

TokenType lexer_skip_identifier(Lexer* self) {
    while (is_ascii_alnum(lexer_peek(self)) || lexer_peek(self) == '_') {
        lexer_advance(self);
    }

    StringView sv = sv_slice(self->src, self->start_i, self->i - self->start_i);

    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (sv_eq_cstr(sv, keywords[i].name)) return keywords[i].tt;
    }

    return TT_ID;
}

void lexer_skip_whitespace(Lexer* self) {
    while (true) {
        char c = lexer_peek(self);

        if (is_ascii_whitespace(c)) {
            lexer_advance(self);
            continue;
        }

        if (c == '#') {
            while (lexer_advance(self) != '\n');
        }

        break;
    }
}

void lexer_setup_start(Lexer* self) {
    self->start_i = self->i;
    self->start_line = self->line;
    self->start_col = self->col;
}

char lexer_peek(Lexer* self) {
    return self->i < self->src.length ? self->src.data[self->i] : '\0';
}

char lexer_advance(Lexer* self) {
    char c = lexer_peek(self);
    self->i++;

    if (c == '\n') {
        self->line_text = sv_first_line(sv_slice_from(self->src, self->i));
        self->line++;
        self->col = 0;
    } else {
        self->col++;
    }

    return c;
}

bool lexer_matches(Lexer* self, char c) {
    if (lexer_peek(self) == c) {
        lexer_advance(self);
        return true;
    }

    return false;
}
