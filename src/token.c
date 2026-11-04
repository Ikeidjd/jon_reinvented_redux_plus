#include "token.h"

const char* tt_repr(TokenType tt) {
    switch (tt) {
        case TT_INT:
            return "TT_INT";
        case TT_PLUS:
            return "TT_PLUS";
        case TT_MINUS:
            return "TT_MINUS";
        case TT_TIMES:
            return "TT_TIMES";
        case TT_DIV:
            return "TT_DIV";
        case TT_MOD:
            return "TT_MOD";
        case TT_EQ:
            return "TT_EQ";
        case TT_COLON:
            return "TT_COLON";
        case TT_SEMI:
            return "TT_SEMI";
        case TT_ID:
            return "TT_ID";
        case TT_KW_LET:
            return "TT_KW_LET";
        case TT_EOF:
            return "TT_EOF";
    }

    return "";
}

const char* tt_error_message_name(TokenType tt) {
    switch (tt) {
        case TT_INT:
            return "integer";
        case TT_PLUS:
            return "'+'";
        case TT_MINUS:
            return "'-'";
        case TT_TIMES:
            return "'*'";
        case TT_DIV:
            return "'/'";
        case TT_MOD:
            return "'%'";
        case TT_EQ:
            return "'='";
        case TT_COLON:
            return "':'";
        case TT_SEMI:
            return "';'";
        case TT_ID:
            return "identifier";
        case TT_KW_LET:
            return "keyword 'let'";
        case TT_EOF:
            return "end of file";
    }

    return "";
}
