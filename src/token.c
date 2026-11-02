#include "token.h"

static const char* tt_repr_array[] = {
    [TT_INT] = "TT_INT",
    [TT_PLUS] = "TT_PLUS",
    [TT_MINUS] = "TT_MINUS",
    [TT_TIMES] = "TT_TIMES",
    [TT_DIV] = "TT_DIV",
    [TT_EOF] = "TT_EOF",
};

static const char* tt_str_array[] = {
    [TT_INT] = "integer",
    [TT_PLUS] = "+",
    [TT_MINUS] = "-",
    [TT_TIMES] = "*",
    [TT_DIV] = "/",
    [TT_EOF] = "end of file",
};

const char* tt_repr(TokenType tt) {
    return tt_repr_array[tt];
}

const char* tt_str(TokenType tt) {
    return tt_str_array[tt];
}
