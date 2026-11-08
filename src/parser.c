#include "parser_.h"
#include "parser_internals.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

bool parse_program(Tokens tokens, Sega_Node* node_buffer, Program* program) {
    Parser self;
    parser_init(&self, tokens, *node_buffer);

    bool b = parser_parse(&self, program);
    *node_buffer = self.node_buffer;

    return b;
}

static Precedence prec_from(TokenType tt) {
    switch (tt) {
        case TT_EQ:
            return PREC_ASSIGN;
        case TT_PLUS:
        case TT_MINUS:
            return PREC_ADD;
        case TT_TIMES:
        case TT_DIV:
        case TT_MOD:
            return PREC_MUL;
        default:
            return PREC_NONE;
    }
}

static bool prec_is_left_associative(Precedence prec) {
    switch (prec) {
        case PREC_NONE:
        case PREC_ADD:
        case PREC_MUL:
            return true;
        case PREC_ASSIGN:
            return false;
    }

    return true;
}

static void parser_init(Parser* self, Tokens tokens, Sega_Node node_buffer) {
    *self = (Parser) {
        .node_buffer = node_buffer,
        .tokens = tokens,
        .i = 0,
    };
}

static ExprNode* parser_new_expr(Parser* self, ExprNode expr) {
    Node node = {
        .type = NODE_EXPR,
        .as.expr = expr,
    };

    sega_append(&self->node_buffer, node);
    return &sega_get_ptr(&self->node_buffer, sega_length(self->node_buffer) - 1)->as.expr;
}

static DeclNode* parser_new_decl(Parser* self, DeclNode decl) {
    Node node = {
        .type = NODE_DECL,
        .as.decl = decl,
    };

    sega_append(&self->node_buffer, node);
    return &sega_get_ptr(&self->node_buffer, sega_length(self->node_buffer) - 1)->as.decl;
}

static TypeNode* parser_new_type(Parser* self, TypeNode type) {
    Node node = {
        .type = NODE_TYPE,
        .as.type = type,
    };

    sega_append(&self->node_buffer, node);
    return &sega_get_ptr(&self->node_buffer, sega_length(self->node_buffer) - 1)->as.type;
}

static bool parser_parse(Parser* self, Program* program) {
    bool success = true;

    while (!parser_matches(self, TT_EOF)) {
        Node* node;
        switch (parser_cur(self).type) {
            case TT_KW_LET:
            case TT_KW_MUT: {
                node = (Node*) parse_decl(self);
            } break;
            default: {
                node = (Node*) parse_expr(self);
                if (node && !parser_expect(self, TT_SEMI)) node = NULL;
            } break;
        }

        if (node) {
            dyna_append(program, node);
        } else {
            success = false;

            // Giving multiple errors on one statement would be a bit of a mess
            while (!parser_matches(self, TT_SEMI) && !parser_matches(self, TT_EOF)) {
                parser_advance(self);
            }
        }
    }

    return success;
}

static TypeNode* parse_type(Parser* self) {
    BasicTypeType type;
    switch (parser_cur(self).type) {
        case TT_KW_INT: {
            type = BASIC_TYPE_INT;
        } break;
        case TT_KW_FLOAT: {
            type = BASIC_TYPE_FLOAT;
        } break;
        case TT_KW_BOOL: {
            type = BASIC_TYPE_BOOL;
        } break;
        case TT_KW_CHAR: {
            type = BASIC_TYPE_CHAR;
        } break;
        default: {
            parser_error_expected_cstr(self, "type name");
            return NULL;
        } break;
    }

    Token token = parser_cur(self);
    parser_advance(self);

    return parser_new_type(self, (TypeNode) {
        .type = TYPE_NODE_BASIC,
        .as.basic_type = {
            .token = token,
            .type = type,
        },
    });
}

static DeclNode* parse_decl(Parser* self) {
    bool mutable;
    if (parser_matches(self, TT_KW_LET)) {
        mutable = false;
    } else if (parser_matches(self, TT_KW_MUT)) {
        mutable = true;
    } else {
        // I don't think this can happen
        parser_error_expected_cstr(self, "keyword 'let' or keyword 'mut'");
        return NULL;
    }

    Token var_name = parser_cur(self);
    if (!parser_expect(self, TT_ID)) return NULL;

    TypeNode* type = NULL;
    if (parser_matches(self, TT_COLON)) {
        type = parse_type(self);
        if (type == NULL) return NULL;
    }

    Token equals_sign = parser_cur(self);
    if (!parser_expect(self, TT_EQ)) return NULL;

    ExprNode* expr = parse_expr(self);
    if (!expr) return NULL;

    if (!parser_expect(self, TT_SEMI)) return NULL;

    return parser_new_decl(self, (DeclNode) {
        .type = DECL_NODE_VAR,
        .as.var_decl = {
            .var_name = var_name,
            .type = type,
            .equals_sign = equals_sign,
            .expr = expr,
            .mutable = mutable,
        },
    });
}

static ExprNode* parse_expr(Parser* self) {
    return parse_prec(self, PREC_NONE);
}

static ExprNode* parse_prec(Parser* self, Precedence prev_prec) {
    if (prec_is_left_associative(prev_prec)) prev_prec++;

    ExprNode* lhs = parse_prefix_expr(self);
    if (!lhs) return NULL;

    Precedence next_prec;
    while ((next_prec = prec_from(parser_cur(self).type)) >= prev_prec) {
        Token op = parser_cur(self);
        parser_advance(self);

        ExprNode* rhs = parse_prec(self, next_prec);
        if (!rhs) return NULL;

        if (op.type == TT_EQ) {
            lhs = parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_ASSIGNMENT,
                .as.assignment = {
                    .token = op,
                    .left = lhs,
                    .right = rhs,
                }
            });

            continue;
        }

        BinOpType op_type;
        switch (op.type) {
            case TT_PLUS: {
                op_type = BIN_ADD;
            } break;
            case TT_MINUS: {
                op_type = BIN_SUB;
            } break;
            case TT_TIMES: {
                op_type = BIN_MUL;
            } break;
            case TT_DIV: {
                op_type = BIN_DIV;
            } break;
            case TT_MOD: {
                op_type = BIN_MOD;
            } break;
            default: {
                fprintf(stderr, "Invalid binary operator '"SV_FMT"'. This should never happen.\n", SV_ARG(op.text));
                exit(-1);
            } break;
        }

        lhs = parser_new_expr(self, (ExprNode) {
            .type = EXPR_NODE_BIN_OP,
            .as.bin_op = {
                .type = op_type,
                .token = op,
                .left = lhs,
                .right = rhs,
            },
        });
    }

    return lhs;
}

static ExprNode* parse_prefix_expr(Parser* self) {
    switch (parser_cur(self).type) {
        case TT_INT: {
            Token token = parser_cur(self);

            i64 i;
            if (!sv_to_i64(token.text, &i)) {
                parser_error(self, "integer "SV_FMT" too large to fit in range", SV_ARG(token.text));
                return NULL;
            }

            parser_advance(self);

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_INT,
                .as.literal = {
                    .token = token,
                    .as.integer = i,
                },
            });
        }
        case TT_FLOAT: {
            Token token = parser_cur(self);

            f64 d;
            if (!sv_to_f64(token.text, &d)) {
                parser_error(self, "could not parse float "SV_FMT"", SV_ARG(token.text));
                return NULL;
            }

            parser_advance(self);

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_FLOAT,
                .as.literal = {
                    .token = token,
                    .as.floating = d,
                },
            });
        }
        case TT_KW_TRUE:
        case TT_KW_FALSE: {
            Token token = parser_cur(self);
            parser_advance(self);

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_BOOL,
                .as.literal = {
                    .token = token,
                    .as.boolean = token.type == TT_KW_TRUE,
                },
            });
        }
        case TT_CHAR: {
            Token token = parser_cur(self);
            parser_advance(self);

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_CHAR,
                .as.literal = {
                    .token = token,
                    .as.character = token.text.data[1], // The text includes the quotes, so indexing with 1 is necessary to get the character
                },
            });
        }
        case TT_ID: {
            Token token = parser_cur(self);
            parser_advance(self);

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_VAR,
                .as.var = {
                    .token = token,
                },
            });
        }
        case TT_MINUS: {
            Token token = parser_cur(self);
            parser_advance(self);

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_UNARY_OP,
                .as.unary_op = {
                    .type = UNARY_MINUS,
                    .token = token,
                    .operand = parse_prefix_expr(self),
                },
            });
        }
        default: {
            parser_error_expected_cstr(self, "expression");
            return NULL;
        }
    }
}

static Token parser_cur(Parser* self) {
    return self->tokens.data[self->i];
}

static void parser_advance(Parser* self) {
    if (parser_cur(self).type != TT_EOF) self->i++;
}

static bool parser_has(Parser* self, TokenType tt) {
    return parser_cur(self).type == tt;
}

static bool parser_matches(Parser* self, TokenType tt) {
    if (parser_has(self, tt)) {
        parser_advance(self);
        return true;
    }

    return false;
}

static bool parser_expect(Parser* self, TokenType tt) {
    if (!parser_matches(self, tt)) {
        parser_error_expected_cstr(self, tt_error_message_name(tt));
        return false;
    }

    return true;
}

static void parser_error(Parser* self, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Syntax error on line %zu: ", parser_cur(self).line_number + 1);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, parser_cur(self));

    va_end(args);
}

static void parser_error_expected(Parser* self, StringView expected) {
    parser_error(self, "expected "SV_FMT", but got %s'"SV_FMT"'", SV_ARG(expected), tt_is_keyword(parser_cur(self).type) ? "keyword " : "", SV_ARG(parser_cur(self).text));
}

static void parser_error_expected_cstr(Parser* self, const char* expected) {
    parser_error(self, "expected %s, but got %s'"SV_FMT"'", expected, tt_is_keyword(parser_cur(self).type) ? "keyword " : "", SV_ARG(parser_cur(self).text));
}
