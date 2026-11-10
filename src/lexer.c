#include "lexer.h"
#include "lexer_internals.h"

#include <stdio.h>
#include <stdarg.h>

#include "char_utils.h"

typedef struct {
    const char* name;
    TokenType tt;
} Keyword;

static Keyword keywords[] = {
    { "let", TT_KW_LET },
    { "mut", TT_KW_MUT },
    { "as", TT_KW_AS },
    { "int", TT_KW_INT },
    { "float", TT_KW_FLOAT },
    { "bool", TT_KW_BOOL },
    { "true", TT_KW_TRUE },
    { "false", TT_KW_FALSE },
    { "char", TT_KW_CHAR },
};

bool lex_program(StringView src, Tokens* token_buf) {
    Lexer self;
    lexer_init(&self, src);

    bool success = true;

    while (true) {
        Token token;

        if (lexer_next(&self, &token)) {
            dyna_append(token_buf, token);
            if (token.type == TT_EOF) break;
        } else {
            success = false;
        }
    }

    return success;
}

static void lexer_init(Lexer* self, StringView src) {
    *self = (Lexer) {
        .src = src,
        .line_text = sv_first_line(src),
        .i = 0,
        .line = 0,
        .col = 0,
    };
}

static bool lexer_next(Lexer* self, Token* out) {
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
            if (is_ascii_digit(lexer_peek(self)) && !lexer_skip_number(self, &tt, lexer_advance(self))) return false;
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
        case '&': {
            tt = lexer_matches(self, '&') ? TT_ANDAND : TT_AND;
        } break;
        case '^': {
            tt = TT_XOR;
        } break;
        case '|': {
            tt = lexer_matches(self, '|') ? TT_OROR : TT_OR;
        } break;
        case '<': {
            tt = lexer_matches(self, '<') ? TT_LTLT : lexer_matches(self, '=') ? TT_LTEQ : TT_LT;
        } break;
        case '>': {
            tt = lexer_matches(self, '>') ? TT_GTGT : lexer_matches(self, '=') ? TT_GTEQ : TT_GT;
        } break;
        case '~': {
            tt = TT_TILDE;
        } break;
        case '!': {
            tt = lexer_matches(self, '=') ? TT_BANG_EQ : TT_BANG;
        } break;
        case '=': {
            tt = lexer_matches(self, '=') ? TT_EQEQ : TT_EQ;
        } break;
        case '(': {
            tt = TT_LPAREN;
        } break;
        case ')': {
            tt = TT_RPAREN;
        } break;
        case '{': {
            tt = TT_LBRACE;
        } break;
        case '}': {
            tt = TT_RBRACE;
        } break;
        case ':': {
            tt = TT_COLON;
        } break;
        case ';': {
            tt = TT_SEMI;
        } break;
        case '\'': {
            tt = TT_CHAR;
            if (!lexer_skip_char(self)) return false;
        } break;
        case '\0': {
            tt = TT_EOF;
        } break;
        default: {
            if (is_ascii_digit(c)) {
                if (!lexer_skip_number(self, &tt, c)) return false;
            } else if (is_ascii_alpha(c) || c == '_') {
                tt = lexer_skip_identifier(self);
            } else {
                goto InvalidCharacter;
            }
        } break;
    }

    *out = lexer_get_cur_token(self, tt);
    return true;

    InvalidCharacter:
    lexer_error(self, "invalid character '%c'", c);
    return false;
}

static bool lexer_skip_number(Lexer* self, TokenType* tt, char c) {
    if (c == '0') {
        char c = lexer_peek(self);

        if (c == 'x' || c == 'o' || c == 'b') {
            *tt = TT_INT;
            lexer_advance(self);
        }

        switch (c) {
            case 'x': {
                if (!is_hex_digit(lexer_peek(self))) {
                    lexer_error(self, "expected hex number after '0x', but got '%c'", lexer_peek(self));
                    return false;
                }

                while (is_hex_digit(lexer_peek(self))) {
                    lexer_advance(self);
                }

                return true;
            }
            case 'o': {
                if (!is_oct_digit(lexer_peek(self))) {
                    lexer_error(self, "expected octal number after '0o', but got '%c'", lexer_peek(self));
                    return false;
                }

                while (is_oct_digit(lexer_peek(self))) {
                    lexer_advance(self);
                }

                return true;
            }
            case 'b': {
                if (!is_bin_digit(lexer_peek(self))) {
                    lexer_error(self, "expected binary number after '0b', but got '%c'", lexer_peek(self));
                    return false;
                }

                while (is_bin_digit(lexer_peek(self))) {
                    lexer_advance(self);
                }

                return true;
            }
        }
    }

    while (is_ascii_digit(lexer_peek(self))) {
        lexer_advance(self);
    }

    if (lexer_matches(self, '.')) {
        if (!is_ascii_digit(lexer_peek(self))) {
            lexer_error(self, "float can't end on a '.'; add a '0' at the end");
            return false;
        }

        while (is_ascii_digit(lexer_peek(self))) {
            lexer_advance(self);
        }

        *tt = TT_FLOAT;
    } else {
        *tt = TT_INT;
    }

    return true;
}

static bool lexer_skip_char(Lexer* self) {
    lexer_advance(self);

    if (!lexer_matches(self, '\'')) {
        lexer_error(self, "unclosed character literal");
        return false;
    }

    return true;
}

static TokenType lexer_skip_identifier(Lexer* self) {
    while (is_ascii_alnum(lexer_peek(self)) || lexer_peek(self) == '_') {
        lexer_advance(self);
    }

    StringView sv = sv_slice(self->src, self->start_i, self->i - self->start_i);

    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (sv_eq_cstr(sv, keywords[i].name)) return keywords[i].tt;
    }

    return TT_ID;
}

static void lexer_skip_whitespace(Lexer* self) {
    while (true) {
        char c = lexer_peek(self);

        if (is_ascii_whitespace(c)) {
            lexer_advance(self);
            continue;
        }

        if (c == '#') {
            while (lexer_advance(self) != '\n');
            continue;
        }

        break;
    }
}

static void lexer_setup_start(Lexer* self) {
    self->start_i = self->i;
    self->start_line = self->line;
    self->start_col = self->col;
}

static Token lexer_get_cur_token(Lexer* self, TokenType tt) {
    return (Token) {
        .type = tt,
        .text = tt == TT_EOF ? sv_lit("") : sv_slice(self->src, self->start_i, self->i - self->start_i),
        .line_text = self->line_text,
        .line_number = self->line,
        .start_col = self->start_col,
        .end_col = self->col,
    };
}

static char lexer_peek(Lexer* self) {
    return self->i < self->src.length ? self->src.data[self->i] : '\0';
}

static char lexer_advance(Lexer* self) {
    char c = lexer_peek(self);
    if (c == '\0') return c;

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

static bool lexer_matches(Lexer* self, char c) {
    if (lexer_peek(self) == c) {
        lexer_advance(self);
        return true;
    }

    return false;
}

static void lexer_error(Lexer* self, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Syntax error on line %zu: ", self->start_line + 1);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, lexer_get_cur_token(self, TT_EOF)); // TT_EOF doesn't matter

    va_end(args);
}
