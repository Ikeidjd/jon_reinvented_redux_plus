#include "type.h"

#include <stdlib.h>

void type_fprint(FILE* file, const Type* type) {
    switch (type->type) {
        case TYPE_BASIC: {
            fprintf(file, "%s", basic_type_to_string(((BasicType*) type)->type));
        } break;
    }
}

hash_t type_hash(Type* type) {
    return type->hash;
}

bool type_eq(Type* a, Type* b) {
    if (a == b) return true;
    if (a == NULL || b == NULL || a->type != b->type) return false;

    switch (a->type) {
        case TYPE_BASIC:
            return ((BasicType*) a)->type == ((BasicType*) b)->type;
    }

    return false;
}

const char* basic_type_to_string(BasicTypeType type) {
    switch (type) {
        case BASIC_TYPE_INT:
            return "int";
        case BASIC_TYPE_FLOAT:
            return "float";
        case BASIC_TYPE_BOOL:
            return "bool";
        case BASIC_TYPE_CHAR:
            return "char";
    }

    return "";
}

#define TYPE_ARENA_SIZE 131072

void type_arena_init(TypeArena* arena) {
    *arena = (TypeArena) {
        .buf = malloc(TYPE_ARENA_SIZE),
        .length = 0,
    };

    hash_table_init(&arena->hash_table, type_hash, type_eq);
}

BasicType* basic_type_new(TypeArena* arena, BasicTypeType basic_type) {
    BasicType type = {
        .header = {
            .type = TYPE_BASIC,
            .hash = (hash_t) basic_type,
        },
        .type = basic_type,
    };

    size_t offset = (alignof(type) - arena->length % alignof(type)) % alignof(type);

    BasicType* out = (BasicType*) (arena->buf + arena->length + offset);
    *out = type;

    BasicType* got = (BasicType*) hash_table_get_or_insert(&arena->hash_table, (Type*) out);

    if (out == got) {
        // If it was inserted
        arena->length += sizeof(type) + offset;
    } else {
        // If it was already present
        out = got;
    }

    return out;
}
