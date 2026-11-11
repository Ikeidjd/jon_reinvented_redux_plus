#pragma once

#include "typedef_utils.h"
#include "dynamic_array.h"

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    OP_BIT_AND,
    OP_BIT_XOR,
    OP_BIT_OR,
    OP_BIT_LSHIFT,
    OP_BIT_RSHIFT,

    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,

    OP_NEGATE,
    OP_BIT_NOT,
    OP_NOT,

    OP_CAST_INT,
    OP_CAST_FLOAT,
    OP_CAST_BOOL,
    OP_CAST_CHAR,

    OP_LOAD_INT,
    OP_LOAD_FLOAT,
    OP_LOAD_BOOL,
    OP_LOAD_CHAR,
    OP_LOAD_MONOSTATE,

    OP_GET_LOCAL,
    OP_SET_LOCAL,

    OP_POP,
    OP_DUP,
    OP_END_BLOCK,

    OP_JUMP,
    OP_JUMP_IF_TRUE,
    OP_JUMP_IF_FALSE,
    OP_LOOP, // Backwards jump

    OP_PRINT,
    OP_PRINTLN,
} Opcode;

const char* opcode_to_string(Opcode opcode);

typedef struct {
    DynamicArray(u8) bytes;
    DynamicArray(size_t) lines;
} Chunk;

void chunk_init(Chunk* self);

void chunk_destroy(Chunk* self);

void chunk_emit(Chunk* self, Opcode opcode, size_t line);

void chunk_emit_args(Chunk* self, Opcode opcode, size_t n, u8 args[n], size_t line);

void chunk_emit_load_int(Chunk* self, i64 i, size_t line);

void chunk_emit_load_float(Chunk* self, f64 d, size_t line);

void chunk_emit_load_bool(Chunk* self, bool b, size_t line);

void chunk_emit_load_char(Chunk* self, char c, size_t line);

void chunk_emit_get_local(Chunk* self, u16 index, size_t line);

void chunk_emit_set_local(Chunk* self, u16 index, size_t line);

size_t chunk_emit_jump(Chunk* self, Opcode opcode, size_t line);

void chunk_patch_jump(Chunk* self, size_t src);

void chunk_emit_loop(Chunk* self, size_t dst, size_t line);

void chunk_disassemble_one(Chunk self, size_t offset);

void chunk_disassemble(Chunk self);
