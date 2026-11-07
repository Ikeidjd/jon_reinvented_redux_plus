#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "print_utils.h"
#include "hash_table.h"

// IMPORTANT: you shouldn't allocate a Type directly, since some places assume that reference equality and value equality are equivalent for it
// Only allocate through TypeArena

typedef enum {
    TYPE_BASIC,
} TypeType;

typedef struct Type {
    TypeType type;
    hash_t hash;
} Type;

void type_fprint(FILE* file, const Type* type);

hash_t type_hash(Type* type);

bool type_eq(Type* a, Type* b);

PRINT_GEN_ALL(const Type*, type)

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

typedef struct {
    char* buf;
    size_t length;
    HashTable_Type hash_table;
} TypeArena;

void type_arena_init(TypeArena* arena);

void type_arena_destroy(TypeArena* arena);

BasicType* basic_type_new(TypeArena* arena, BasicTypeType basic_type);
