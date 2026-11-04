#pragma once

typedef enum {
    BASIC_TYPE_INT,
    BASIC_TYPE_FLOAT,
    BASIC_TYPE_BOOL,
    BASIC_TYPE_CHAR,
} BasicType;

const char* basic_type_to_string(BasicType type);
