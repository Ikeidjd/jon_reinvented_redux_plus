#pragma once

#include <stdio.h>
#include <stddef.h>

#include "print_utils.h"

typedef enum {
    TYPE_BASIC,
} TypeType;

typedef struct {
    TypeType type;
} Type;

void type_fprint(FILE* file, const Type* type);

PRINT_GEN_ALL(const Type*, type);

typedef enum {
    BASIC_TYPE_INT,
    BASIC_TYPE_FLOAT,
    BASIC_TYPE_BOOL,
    BASIC_TYPE_CHAR,
} BasicTypeType;

typedef struct {
    Type header;
    BasicTypeType type;
} BasicType;

const char* basic_type_to_string(BasicTypeType type);

#define TYPE_ALLOCATOR_SIZE 131072

typedef struct {
    size_t length;
    char buf[TYPE_ALLOCATOR_SIZE];
} TypeArena;

void type_arena_init(TypeArena* arena);

BasicType* basic_type_new(TypeArena* arena, BasicTypeType basic_type);
