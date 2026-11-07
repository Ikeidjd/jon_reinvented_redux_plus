#pragma once

#include "typedef_utils.h"
#include "dynamic_array.h"

typedef unsigned char u8;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    OP_LOAD_INT,
    OP_LOAD_FLOAT,
    OP_LOAD_BOOL,
    OP_LOAD_CHAR,

    OP_GET_LOCAL,
    OP_SET_LOCAL,

    OP_POP,
} Opcode;

const char* opcode_to_string(Opcode opcode);

typedef DynamicArray(Opcode) Chunk;

void chunk_emit(Chunk* self, Opcode opcode);

void chunk_emit_args(Chunk* self, Opcode opcode, size_t n, u8 args[n]);

void chunk_emit_load_int(Chunk* self, i64 i);

void chunk_emit_load_float(Chunk* self, f64 d);

void chunk_emit_load_bool(Chunk* self, bool b);

void chunk_emit_load_char(Chunk* self, char c);

void chunk_emit_get_local(Chunk* self, u16 index);

void chunk_emit_set_local(Chunk* self, u16 index);

void chunk_disassemble(Chunk self);
