#include "value.h"

#include <assert.h>

#define MAKE_VALUE_BIN_OP_INT_AND_FLOAT(op_name, op_symbol) \
Value value_ ## op_name(Value a, Value b) { \
    assert(a.type == b.type && "Values must be the same type to"#op_name); \
\
    switch (a.type) { \
        case VALUE_INT: \
            return (Value) { \
                .type = a.type, \
                .as.integer = a.as.integer op_symbol b.as.integer, \
            }; \
        case VALUE_FLOAT: \
            return (Value) { \
                .type = a.type, \
                .as.floating = a.as.floating op_symbol b.as.floating, \
            }; \
        case VALUE_BOOL: \
            assert(false && "Can't "#op_name" bool"); \
        case VALUE_CHAR: \
            assert(false && "Can't "#op_name" char"); \
    } \
\
    assert(false && "Unreachable"); \
}

#define MAKE_VALUE_BIN_OP_INT(op_name, op_symbol) \
Value value_ ## op_name(Value a, Value b) { \
    assert(a.type == b.type && "Values must be the same type to"#op_name); \
\
    switch (a.type) { \
        case VALUE_INT: \
            return (Value) { \
                .type = a.type, \
                .as.integer = a.as.integer op_symbol b.as.integer, \
            }; \
        case VALUE_FLOAT: \
            assert(false && "Can't "#op_name" float"); \
        case VALUE_BOOL: \
            assert(false && "Can't "#op_name" bool"); \
        case VALUE_CHAR: \
            assert(false && "Can't "#op_name" char"); \
    } \
\
    assert(false && "Unreachable"); \
}

MAKE_VALUE_BIN_OP_INT_AND_FLOAT(add, +)
MAKE_VALUE_BIN_OP_INT_AND_FLOAT(sub, -)
MAKE_VALUE_BIN_OP_INT_AND_FLOAT(mul, *)
MAKE_VALUE_BIN_OP_INT_AND_FLOAT(div, /)
MAKE_VALUE_BIN_OP_INT(mod, %)

MAKE_VALUE_BIN_OP_INT(bit_and, &)
MAKE_VALUE_BIN_OP_INT(bit_xor, ^)
MAKE_VALUE_BIN_OP_INT(bit_or, |)
MAKE_VALUE_BIN_OP_INT(bit_lshift, <<)
MAKE_VALUE_BIN_OP_INT(bit_rshift, >>)

#define MAKE_VALUE_EQ(op_name, op_symbol) \
bool value_ ## op_name(Value a, Value b) { \
    assert(a.type == b.type && "Values must be the same type to"#op_name); \
\
    switch (a.type) { \
        case VALUE_INT: \
            return a.as.integer op_symbol b.as.integer; \
        case VALUE_FLOAT: \
            return a.as.floating op_symbol b.as.floating; \
        case VALUE_BOOL: \
            return a.as.boolean op_symbol b.as.boolean; \
        case VALUE_CHAR: \
            return a.as.character op_symbol b.as.character; \
    } \
\
    assert(false && "Unreachable"); \
}

#define MAKE_VALUE_COMP(op_name, op_symbol) \
bool value_ ## op_name(Value a, Value b) { \
    assert(a.type == b.type && "Values must be the same type to"#op_name); \
\
    switch (a.type) { \
        case VALUE_INT: \
            return a.as.integer op_symbol b.as.integer; \
        case VALUE_FLOAT: \
            return a.as.floating op_symbol b.as.floating; \
        case VALUE_BOOL: \
            assert(false && "Can't "#op_name" bool"); \
        case VALUE_CHAR: \
            return a.as.character op_symbol b.as.character; \
    } \
\
    assert(false && "Unreachable"); \
}

MAKE_VALUE_EQ(eq, ==);
MAKE_VALUE_EQ(ne, !=);
MAKE_VALUE_COMP(lt, <);
MAKE_VALUE_COMP(le, <=);
MAKE_VALUE_COMP(gt, >);
MAKE_VALUE_COMP(ge, >=);

#define MAKE_VALUE_UNARY_OP_INT_AND_FLOAT(op_name, op_symbol) \
Value value_ ## op_name(Value value) { \
    switch (value.type) { \
        case VALUE_INT: \
            return (Value) { \
                .type = value.type, \
                .as.integer = op_symbol value.as.integer, \
            }; \
        case VALUE_FLOAT: \
            return (Value) { \
                .type = value.type, \
                .as.floating = op_symbol value.as.floating, \
            }; \
        case VALUE_BOOL: \
            assert(false && "Can't "#op_name" bool"); \
        case VALUE_CHAR: \
            assert(false && "Can't "#op_name" char"); \
    } \
\
    assert(false && "Unreachable"); \
}

#define MAKE_VALUE_UNARY_OP_INT(op_name, op_symbol) \
Value value_ ## op_name(Value value) { \
    switch (value.type) { \
        case VALUE_INT: \
            return (Value) { \
                .type = value.type, \
                .as.integer = op_symbol value.as.integer, \
            }; \
        case VALUE_FLOAT: \
            assert(false && "Can't "#op_name" float"); \
        case VALUE_BOOL: \
            assert(false && "Can't "#op_name" bool"); \
        case VALUE_CHAR: \
            assert(false && "Can't "#op_name" char"); \
    } \
\
    assert(false && "Unreachable"); \
}

#define MAKE_VALUE_UNARY_OP_BOOL(op_name, op_symbol) \
Value value_ ## op_name(Value value) { \
    switch (value.type) { \
        case VALUE_INT: \
            assert(false && "Can't "#op_name" int"); \
        case VALUE_FLOAT: \
            assert(false && "Can't "#op_name" float"); \
        case VALUE_BOOL: \
            return (Value) { \
                .type = value.type, \
                .as.boolean = op_symbol value.as.boolean, \
            }; \
        case VALUE_CHAR: \
            assert(false && "Can't "#op_name" char"); \
    } \
\
    assert(false && "Unreachable"); \
}

MAKE_VALUE_UNARY_OP_INT_AND_FLOAT(negate, -)
MAKE_VALUE_UNARY_OP_INT(bit_not, ~)
MAKE_VALUE_UNARY_OP_BOOL(not, !)

Value value_cast_int(Value value) {
    switch (value.type) {
        case VALUE_INT:
            return value;
        case VALUE_FLOAT:
            return (Value) {
                .type = VALUE_INT,
                .as.integer = value.as.floating > (f64) INT64_MAX ? INT64_MAX : value.as.floating < INT64_MIN ? INT64_MIN : (i64) value.as.floating,
            };
        case VALUE_BOOL:
            return (Value) {
                .type = VALUE_INT,
                .as.integer = (i64) value.as.boolean,
            };
        case VALUE_CHAR:
            return (Value) {
                .type = VALUE_INT,
                .as.integer = (i64) value.as.character,
            };
    }

    assert(false && "Unreachable");
}

Value value_cast_float(Value value) {
    switch (value.type) {
        case VALUE_INT:
            return (Value) {
                .type = VALUE_FLOAT,
                .as.floating = (f64) value.as.integer,
            };
        case VALUE_FLOAT:
            return value;
        case VALUE_BOOL:
            assert(false && "Can't cast bool to float");
        case VALUE_CHAR:
            assert(false && "Can't cast char to float");
    }

    assert(false && "Unreachable");
}

Value value_cast_bool(Value value) {
    switch (value.type) {
        case VALUE_INT:
            return (Value) {
                .type = VALUE_BOOL,
                .as.boolean = (bool) value.as.integer,
            };
        case VALUE_FLOAT:
            assert(false && "Can't cast float to bool");
        case VALUE_BOOL:
            return value;
        case VALUE_CHAR:
            assert(false && "Can't cast char to bool");
    }

    assert(false && "Unreachable");
}

Value value_cast_char(Value value) {
    switch (value.type) {
        case VALUE_INT:
            return (Value) {
                .type = VALUE_CHAR,
                .as.character = (char) value.as.integer,
            };
        case VALUE_FLOAT:
            assert(false && "Can't cast float to char");
        case VALUE_BOOL:
            assert(false && "Can't cast bool to char");
        case VALUE_CHAR:
            return value;
    }

    assert(false && "Unreachable");
}

void value_fprint(FILE* file, Value value) {
    switch (value.type) {
        case VALUE_INT: {
            fprintf(file, I64_FMT, value.as.integer);
        } break;
        case VALUE_FLOAT: {
            fprintf(file, "%f", value.as.floating);
        } break;
        case VALUE_BOOL: {
            fprintf(file, "%s", value.as.boolean ? "true" : "false");
        } break;
        case VALUE_CHAR: {
            fprintf(file, "%c", value.as.character);
        } break;
    }
}

void value_repr_fprint(FILE* file, Value value) {
    switch (value.type) {
        case VALUE_INT: {
            fprintf(file, I64_FMT, value.as.integer);
        } break;
        case VALUE_FLOAT: {
            fprintf(file, "%f", value.as.floating);
        } break;
        case VALUE_BOOL: {
            fprintf(file, "%s", value.as.boolean ? "true" : "false");
        } break;
        case VALUE_CHAR: {
            fprintf(file, "'%c'", value.as.character);
        } break;
    }
}
