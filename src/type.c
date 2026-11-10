#include "type.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void type_fprint(FILE* file, const Type* type) {
    if (type == NULL) {
        fprintf(file, "<error type>");
        return;
    }

    switch (type->type) {
        case TYPE_PRIMITIVE: {
            fprintf(file, "%s", primitive_type_to_string(((PrimitiveType*) type)->type));
        } break;
        case TYPE_MONOSTATE: {
            fprintf(file, "monostate");
        } break;
    }
}

hash_t type_hash(Type* type) {
    assert(type && "Don't try to hash null types");
    return type->hash;
}

bool type_eq(Type* a, Type* b) {
    if (a == b) return true;
    if (a == NULL || b == NULL || a->type != b->type) return false;

    switch (a->type) {
        case TYPE_PRIMITIVE:
            return ((PrimitiveType*) a)->type == ((PrimitiveType*) b)->type;
        case TYPE_MONOSTATE:
            return true;
    }
    
    assert(false && "Unreachable");
}

const char* primitive_type_to_string(PrimitiveTypeType type) {
    switch (type) {
        case PRIMITIVE_TYPE_INT:
            return "int";
        case PRIMITIVE_TYPE_FLOAT:
            return "float";
        case PRIMITIVE_TYPE_BOOL:
            return "bool";
        case PRIMITIVE_TYPE_CHAR:
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

void type_arena_destroy(TypeArena* arena) {
    free(arena->buf);
    hash_table_destroy(&arena->hash_table);
}

static Type* _type_arena_get_or_insert(TypeArena* arena, Type* type, size_t size, size_t offset) {
    Type* out = (Type*) (arena->buf + arena->length + offset);
    memcpy(out, type, size);

    Type* got = hash_table_get_or_insert(&arena->hash_table, out);

    if (out == got) {
        // If it was inserted
        arena->length += size + offset;
    } else {
        // If it was already present
        out = got;
    }

    return out;
}

#define type_arena_get_or_insert(arena, type) _type_arena_get_or_insert(arena, (Type*) &(type), sizeof(type), (alignof(type) - (arena)->length % alignof(type)) % alignof(type))

PrimitiveType* primitive_type_new(TypeArena* arena, PrimitiveTypeType primitive_type) {
    PrimitiveType type = {
        .header = {
            .type = TYPE_PRIMITIVE,
            .hash = (hash_t) primitive_type + 1, // monostate is 0, so we add 1 here
        },
        .type = primitive_type,
    };

    return (PrimitiveType*) type_arena_get_or_insert(arena, type);
}

Type* monostate_type_new(TypeArena* arena) {
    Type type = {
        .type = TYPE_MONOSTATE,
        .hash = 0,
    };

    return type_arena_get_or_insert(arena, type);
}
