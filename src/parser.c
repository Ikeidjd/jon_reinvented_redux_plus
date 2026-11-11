#include "parser_.h"
#include "parser_internals.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

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
        case TT_OROR:
            return PREC_OR;
        case TT_ANDAND:
            return PREC_AND;
        case TT_EQEQ:
        case TT_BANG_EQ:
        case TT_LT:
        case TT_LTEQ:
        case TT_GT:
        case TT_GTEQ:
            return PREC_COMPARISON;
        case TT_OR:
            return PREC_BIT_OR;
        case TT_XOR:
            return PREC_BIT_XOR;
        case TT_AND:
            return PREC_BIT_AND;
        case TT_LTLT:
        case TT_GTGT:
            return PREC_BIT_SHIFT;
        case TT_PLUS:
        case TT_MINUS:
            return PREC_ADD;
        case TT_TIMES:
        case TT_DIV:
        case TT_MOD:
            return PREC_MUL;
        case TT_KW_AS:
            return PREC_CAST;
        default:
            return PREC_NONE;
    }
}

static Associativity prec_get_associativity(Precedence prec) {
    switch (prec) {
        case PREC_NONE:
        case PREC_BIT_OR:
        case PREC_OR:
        case PREC_AND:
        case PREC_BIT_XOR:
        case PREC_BIT_AND:
        case PREC_BIT_SHIFT:
        case PREC_ADD:
        case PREC_MUL:
        case PREC_CAST:
        case PREC_UNARY_PREFIX:
            return ASS_LEFT;
        case PREC_ASSIGN:
            return ASS_RIGHT;
        case PREC_COMPARISON:
            return ASS_PAREN;
    }

    assert(false && "Unreachable");
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

static StatNode* parser_new_stat(Parser* self, StatNode stat) {
    Node node = {
        .type = NODE_STAT,
        .as.stat = stat,
    };

    sega_append(&self->node_buffer, node);
    return &sega_get_ptr(&self->node_buffer, sega_length(self->node_buffer) - 1)->as.stat;
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
    return parse_nodes(self, (Dyna_Node*) program, NULL);
}

static bool parse_nodes(Parser* self, Dyna_Node* nodes, BlockNode* block) {
    bool success = true;

    while (true) {
        if (parser_has(self, TT_EOF) || (block && parser_has(self, TT_RBRACE))) break;

        Node* node = NULL;
        switch (parser_cur(self).type) {
            case TT_KW_LET:
            case TT_KW_MUT: {
                node = (Node*) parse_decl(self);
            } break;
            case TT_KW_WHILE: {
                node = (Node*) parse_while_loop(self);
            } break;
            case TT_KW_PRINT:
            case TT_KW_PRINTLN: {
                Token token = parser_cur(self);
                parser_advance(self);

                ExprNode* expr = parse_expr(self);
                if (!expr || !parser_expect(self, TT_SEMI)) break;

                node = (Node*) parser_new_stat(self, (StatNode) {
                    .type = STAT_NODE_PRINT,
                    .as.print = {
                        .token = token,
                        .expr = expr,
                        .is_println = token.type == TT_KW_PRINTLN,
                    },
                });
            } break;
            default: {
                ExprNode* expr = parse_expr(self);
                if (!expr) break;

                // Not parser_matches because the part after the loop handles that
                if (block && parser_has(self, TT_RBRACE)) {
                    node = (Node*) expr;
                    block->has_value = true;
                    break;
                }

                // Block and if do not require a semicolon on their own
                if (expr->type == EXPR_NODE_BLOCK || expr->type == EXPR_NODE_IF_ELSE || parser_expect(self, TT_SEMI)) {
                    node = (Node*) expr;
                }
            } break;
        }

        if (node) {
            dyna_append(nodes, node);
        } else {
            success = false;

            // Giving multiple errors on one statement would be a bit of a mess
            while (true) {
                if (parser_matches(self, TT_SEMI) || parser_matches(self, TT_EOF) || (block && parser_has(self, TT_RBRACE))) break;
                parser_advance(self);
            }
        }
    }

    if (block) {
        block->rbrace = parser_cur(self);
        success &= parser_expect(self, TT_RBRACE);
    }

    return success;
}

static BlockNode* parse_block(Parser* self) {
    if (!parser_expect(self, TT_LBRACE)) return NULL;

    BlockNode* block = (BlockNode*) parser_new_expr(self, (ExprNode) {
        .type = EXPR_NODE_BLOCK,
        .as.block = {
            .has_value = false,
        },
    });

    dyna_init(&block->nodes);
    return parse_nodes(self, (Dyna_Node*) &block->nodes, block) ? block : NULL;
}

static TypeNode* parse_type(Parser* self) {
    Token token = parser_cur(self);
    PrimitiveTypeType type;

    switch (parser_cur(self).type) {
        case TT_KW_INT: {
            type = PRIMITIVE_TYPE_INT;
        } break;
        case TT_KW_FLOAT: {
            type = PRIMITIVE_TYPE_FLOAT;
        } break;
        case TT_KW_BOOL: {
            type = PRIMITIVE_TYPE_BOOL;
        } break;
        case TT_KW_CHAR: {
            type = PRIMITIVE_TYPE_CHAR;
        } break;
        case TT_KW_MONOSTATE: {
            parser_advance(self);

            return parser_new_type(self, (TypeNode) {
                .type = TYPE_NODE_MONOSTATE,
                .as.monostate_type = {
                    .token = token,
                },
            });
        }
        default: {
            parser_error_expected_cstr(self, "type name");
            return NULL;
        }
    }

    parser_advance(self);

    return parser_new_type(self, (TypeNode) {
        .type = TYPE_NODE_PRIMITIVE,
        .as.primitive_type = {
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

static WhileLoopNode* parse_while_loop(Parser* self) {
    Token token = parser_cur(self);
    parser_advance(self);

    ExprNode* condition = parse_expr(self);
    if (!condition) return NULL;

    BlockNode* body = parse_block(self);
    if (!body) return NULL;

    return (WhileLoopNode*) parser_new_stat(self, (StatNode) {
        .type = STAT_NODE_WHILE_LOOP,
        .as.while_loop = {
            .token = token,
            .condition = condition,
            .body = body,
        },
    });
}

static bool tt_to_bin_op_type(TokenType tt, BinOpType* out) {
    switch (tt) {
        case TT_PLUS: {
            *out = BIN_ADD;
        } break;
        case TT_MINUS: {
            *out = BIN_SUB;
        } break;
        case TT_TIMES: {
            *out = BIN_MUL;
        } break;
        case TT_DIV: {
            *out = BIN_DIV;
        } break;
        case TT_MOD: {
            *out = BIN_MOD;
        } break;
        case TT_AND: {
            *out = BIN_BIT_AND;
        } break;
        case TT_XOR: {
            *out = BIN_BIT_XOR;
        } break;
        case TT_OR: {
            *out = BIN_BIT_OR;
        } break;
        case TT_LTLT: {
            *out = BIN_BIT_LSHIFT;
        } break;
        case TT_GTGT: {
            *out = BIN_BIT_RSHIFT;
        } break;
        case TT_EQEQ: {
            *out = BIN_EQ;
        } break;
        case TT_BANG_EQ: {
            *out = BIN_NE;
        } break;
        case TT_LT: {
            *out = BIN_LT;
        } break;
        case TT_LTEQ: {
            *out = BIN_LE;
        } break;
        case TT_GT: {
            *out = BIN_GT;
        } break;
        case TT_GTEQ: {
            *out = BIN_GE;
        } break;
        default: {
            return false;
        }
    }

    return true;
}

static bool tt_to_logical_op_type(TokenType tt, LogicalOpType* out) {
    switch (tt) {
        case TT_ANDAND: {
            *out = LOG_AND;
        } break;
        case TT_OROR: {
            *out = LOG_OR;
        } break;
        default: {
            return false;
        }
    }

    return true;
}

static UnaryOpType tt_to_unary_op_type(TokenType tt) {
    switch (tt) {
        case TT_MINUS:
            return UNARY_NEGATE;
        case TT_TILDE:
            return UNARY_BIT_NOT;
        case TT_BANG:
            return UNARY_NOT;
        default:
            fprintf(stderr, "Invalid unary operator %s. This should never happen.\n", tt_error_message_name(tt));
            assert(false && "Invalid unary operator");
    }
}

static ExprNode* parse_expr(Parser* self) {
    return parse_prec(self, PREC_NONE);
}

static ExprNode* parse_prec(Parser* self, Precedence prev_prec) {
    Associativity ass = prec_get_associativity(prev_prec);
    if (ass == ASS_LEFT) prev_prec++;

    ExprNode* lhs = parse_prefix_expr(self);
    if (!lhs) return NULL;

    Precedence next_prec;
    while ((next_prec = prec_from(parser_cur(self).type)) >= prev_prec) {
        if (next_prec == prev_prec && ass == ASS_PAREN) {
            parser_error(self, "need parentheses to determine associativity for operation '"SV_FMT"'", SV_ARG(parser_cur(self).text));
            return NULL;
        }

        Token op = parser_cur(self);
        parser_advance(self);

        if (op.type == TT_KW_AS) {
            TypeNode* type = parse_type(self);
            if (!type) return NULL;

            lhs = parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_CAST,
                .as.cast = {
                    .token = op,
                    .expr = lhs,
                    .type = type,
                },
            });

            continue;
        }

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

        BinOpType bin_op_type;
        if (tt_to_bin_op_type(op.type, &bin_op_type)) {
            lhs = parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_BIN_OP,
                .as.bin_op = {
                    .type = bin_op_type,
                    .token = op,
                    .left = lhs,
                    .right = rhs,
                },
            });

            continue;
        }

        LogicalOpType logical_op_type;
        if (tt_to_logical_op_type(op.type, &logical_op_type)) {
            lhs = parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_LOGICAL_OP,
                .as.logical_op = {
                    .type = logical_op_type,
                    .token = op,
                    .left = lhs,
                    .right = rhs,
                },
            });

            continue;
        }

        fprintf(stderr, "Invalid binary or logical operator %s. This should never happen.\n", tt_error_message_name(op.type));
        assert(false && "Invalid binary or logical operator");
    }

    return lhs;
}

static ExprNode* parse_prefix_expr(Parser* self) {
    switch (parser_cur(self).type) {
        case TT_INT: {
            Token token = parser_cur(self);

            bool negative = false;
            StringView number_text = token.text;

            if (number_text.data[0] == '-') {
                negative = true;
                number_text = sv_slice_from(number_text, 1);
            }

            u64 unsigned_integer;
            SvToIntResult (*sv_to_int)(StringView sv, u64* out) = sv_to_u64;

            if (number_text.length > 2) {
                StringView prefix = sv_slice_until(number_text, 2);
                StringView possible_number_text = sv_slice_from(number_text, 2);

                if (sv_eq_cstr(prefix, "0x")) {
                    sv_to_int = sv_hex_to_u64;
                } else if (sv_eq_cstr(prefix, "0o")) {
                    sv_to_int = sv_oct_to_u64;
                } else if (sv_eq_cstr(prefix, "0b")) {
                    sv_to_int = sv_bin_to_u64;
                } else {
                    possible_number_text = number_text;
                }

                number_text = possible_number_text;
            }

            SvToIntResult result = sv_to_int(number_text, &unsigned_integer);
            assert(result != SV_TO_INT_INVALID_DIGIT && "Invalid digits should have been caught earlier");

            if (result == SV_TO_INT_TOO_LARGE || unsigned_integer > (u64) INT64_MAX + 1 || (!negative && unsigned_integer > INT64_MAX)) {
                parser_error(self, "integer "SV_FMT" too large to fit in range", SV_ARG(token.text));
                return NULL;
            }

            i64 integer = (i64) unsigned_integer;
            if (negative) integer = -integer;

            parser_advance(self);

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_INT,
                .as.literal = {
                    .token = token,
                    .as.integer = integer,
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
        case TT_MINUS:
        case TT_TILDE:
        case TT_BANG: {
            Token token = parser_cur(self);
            parser_advance(self);

            ExprNode* operand = parse_prec(self, PREC_UNARY_PREFIX);
            if (!operand) return NULL;

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_UNARY_OP,
                .as.unary_op = {
                    .type = tt_to_unary_op_type(token.type),
                    .token = token,
                    .operand = operand,
                },
            });
        }
        case TT_LPAREN: {
            parser_advance(self);
            ExprNode* expr = parse_expr(self);
            if (!expr || !parser_expect(self, TT_RPAREN)) return NULL;
            return expr;
        }
        case TT_LBRACE: {
            return (ExprNode*) parse_block(self);
        }
        case TT_KW_IF: {
            Token token = parser_cur(self);
            parser_advance(self);

            ExprNode* condition = parse_expr(self);
            if (!condition) return NULL;

            BlockNode* if_branch = parse_block(self);
            if (!if_branch) return NULL;

            ExprNode* else_branch = NULL;

            if (parser_matches(self, TT_KW_ELSE)) {
                if (parser_has(self, TT_KW_IF) || parser_has(self, TT_LBRACE)) {
                    else_branch = parse_prefix_expr(self);
                    if (!else_branch) return NULL;
                } else {
                    parser_error_expected_cstr(self, "keyword 'if' or '{'");
                    return NULL;
                }
            }

            return parser_new_expr(self, (ExprNode) {
                .type = EXPR_NODE_IF_ELSE,
                .as.if_else = {
                    .token = token,
                    .condition = condition,
                    .if_branch = if_branch,
                    .else_branch = else_branch,
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

static void parser_error_at(Parser* self, Token token, const char* fmt, ...) {
    (void) self;

    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Syntax error on line %zu: ", token.line_number + 1);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ".\n");

    token_show_in_line(stderr, token);

    va_end(args);
}

static void parser_error_expected(Parser* self, StringView expected) {
    parser_error(self, "expected "SV_FMT", but got %s'"SV_FMT"'", SV_ARG(expected), tt_is_keyword(parser_cur(self).type) ? "keyword " : "", SV_ARG(parser_cur(self).text));
}

static void parser_error_expected_cstr(Parser* self, const char* expected) {
    parser_error(self, "expected %s, but got %s'"SV_FMT"'", expected, tt_is_keyword(parser_cur(self).type) ? "keyword " : "", SV_ARG(parser_cur(self).text));
}
