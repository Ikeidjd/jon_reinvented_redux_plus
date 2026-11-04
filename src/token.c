#include "token.h"

const char* tt_repr(TokenType tt) {
    switch (tt) {
        case TT_INT:
            return "TT_INT";
        case TT_FLOAT:
            return "TT_FLOAT";
        case TT_CHAR:
            return "TT_CHAR";
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
        case TT_KW_INT:
            return "TT_KW_INT";
        case TT_KW_FLOAT:
            return "TT_KW_FLOAT";
        case TT_KW_BOOL:
            return "TT_KW_BOOL";
        case TT_KW_TRUE:
            return "TT_KW_TRUE";
        case TT_KW_FALSE:
            return "TT_KW_FALSE";
        case TT_KW_CHAR:
            return "TT_KW_CHAR";
        case TT_EOF:
            return "TT_EOF";
    }

    return "";
}

const char* tt_error_message_name(TokenType tt) {
    switch (tt) {
        case TT_INT:
            return "integer";
        case TT_FLOAT:
            return "float";
        case TT_CHAR:
            return "char";
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
        case TT_KW_INT:
            return "keyword 'int'";
        case TT_KW_FLOAT:
            return "keyword 'float'";
        case TT_KW_BOOL:
            return "keyword 'bool'";
        case TT_KW_TRUE:
            return "keyword 'true'";
        case TT_KW_FALSE:
            return "keyword 'false'";
        case TT_KW_CHAR:
            return "keyword 'char'";
        case TT_EOF:
            return "end of file";
    }

    return "";
}

bool tt_is_keyword(TokenType tt) {
    switch (tt) {
        case TT_INT:
        case TT_FLOAT:
        case TT_CHAR:
        case TT_PLUS:
        case TT_MINUS:
        case TT_TIMES:
        case TT_DIV:
        case TT_MOD:
        case TT_EQ:
        case TT_COLON:
        case TT_SEMI:
        case TT_ID:
        case TT_EOF:
            return false;
        case TT_KW_LET:
        case TT_KW_INT:
        case TT_KW_FLOAT:
        case TT_KW_BOOL:
        case TT_KW_TRUE:
        case TT_KW_FALSE:
        case TT_KW_CHAR:
            return true;
    }

    return false;
}
