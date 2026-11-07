#include "type.h"

void type_fprint(FILE* file, const Type* type) {
    switch (type->type) {
        case TYPE_BASIC: {
            fprintf(file, "%s", basic_type_to_string(((BasicType*) type)->type));
        } break;
    }
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

void type_arena_init(TypeArena* arena) {
    arena->length = 0;
}

BasicType* basic_type_new(TypeArena* arena, BasicTypeType basic_type) {
    BasicType type = {
        .header = {
            .type = TYPE_BASIC,
        },
        .type = basic_type,
    };

    size_t offset = (alignof(type) - arena->length % alignof(type)) % alignof(type);
    BasicType* out = (BasicType*) (arena->buf + arena->length + offset);
    arena->length += sizeof(type) + offset;

    *out = type;
    return out;
}
