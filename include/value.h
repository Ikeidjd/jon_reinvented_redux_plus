#pragma once

#include <stdio.h>

#include "typedef_utils.h"
#include "print_utils.h"

typedef enum {
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_BOOL,
    VALUE_CHAR,
} ValueType;

typedef struct {
    union {
        i64 integer;
        f64 floating;
        bool boolean;
        char character;
    } as;
    ValueType type;
} Value;

Value value_add(Value a, Value b);
Value value_sub(Value a, Value b);
Value value_mul(Value a, Value b);
Value value_div(Value a, Value b);
Value value_mod(Value a, Value b);

Value value_bit_and(Value a, Value b);
Value value_bit_xor(Value a, Value b);
Value value_bit_or(Value a, Value b);
Value value_bit_lshift(Value a, Value b);
Value value_bit_rshift(Value a, Value b);

bool value_eq(Value a, Value b);
bool value_ne(Value a, Value b);
bool value_lt(Value a, Value b);
bool value_le(Value a, Value b);
bool value_gt(Value a, Value b);
bool value_ge(Value a, Value b);

Value value_negate(Value value);
Value value_bit_not(Value value);

void value_fprint(FILE* file, Value value);
PRINT_GEN_ALL(Value, value)

void value_repr_fprint(FILE* file, Value value);
PRINT_GEN_ALL(Value, value_repr)
