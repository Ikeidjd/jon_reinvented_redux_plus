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

void value_fprint(FILE* file, Value value);

PRINT_GEN_ALL(Value, value)

void value_repr_fprint(FILE* file, Value value);

PRINT_GEN_ALL(Value, value_repr)
