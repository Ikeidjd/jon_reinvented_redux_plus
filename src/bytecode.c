#include "bytecode.h"

#include <stdio.h>

#include "little_endian_utils.h"

const char* opcode_to_string(Opcode opcode) {
    switch (opcode) {
        case OP_ADD:
            return "OP_ADD";
        case OP_SUB:
            return "OP_SUB";
        case OP_MUL:
            return "OP_MUL";
        case OP_DIV:
            return "OP_DIV";
        case OP_MOD:
            return "OP_MOD";
        case OP_BIT_AND:
            return "OP_BIT_AND";
        case OP_BIT_XOR:
            return "OP_BIT_XOR";
        case OP_BIT_OR:
            return "OP_BIT_OR";
        case OP_BIT_LSHIFT:
            return "OP_BIT_LSHIFT";
        case OP_BIT_RSHIFT:
            return "OP_BIT_RSHIFT";
        case OP_NEGATE:
            return "OP_NEGATE";
        case OP_BIT_NOT:
            return "OP_BIT_NOT";
        case OP_LOAD_INT:
            return "OP_LOAD_INT";
        case OP_LOAD_FLOAT:
            return "OP_LOAD_FLOAT";
        case OP_LOAD_BOOL:
            return "OP_LOAD_BOOL";
        case OP_LOAD_CHAR:
            return "OP_LOAD_CHAR";
        case OP_GET_LOCAL:
            return "OP_GET_LOCAL";
        case OP_SET_LOCAL:
            return "OP_SET_LOCAL";
        case OP_POP:
            return "OP_POP";
    }

    return "";
}

void chunk_emit(Chunk* self, Opcode opcode) {
    dyna_append(self, opcode);
}

void chunk_emit_args(Chunk* self, Opcode opcode, size_t n, u8 args[n]) {
    chunk_emit(self, opcode);

    for (size_t i = 0; i < n; i++) {
        dyna_append(self, args[i]);
    }
}

void chunk_emit_load_int(Chunk* self, i64 i) {
    u8 buf[sizeof(i)];
    to_le_u64((u64) i, buf);
    chunk_emit_args(self, OP_LOAD_INT, sizeof(i), buf);
}

void chunk_emit_load_float(Chunk* self, f64 f) {
    u8 buf[sizeof(f)];
    to_le_f64(f, buf);
    chunk_emit_args(self, OP_LOAD_FLOAT, sizeof(f), buf);
}

void chunk_emit_load_bool(Chunk* self, bool b) {
    chunk_emit_args(self, OP_LOAD_BOOL, sizeof(b), (u8*) &b);
}

void chunk_emit_load_char(Chunk* self, char c) {
    chunk_emit_args(self, OP_LOAD_CHAR, sizeof(c), (u8*) &c);
}

void chunk_emit_get_local(Chunk* self, u16 index) {
    u8 buf[sizeof(index)];
    to_le_u16(index, buf);
    chunk_emit_args(self, OP_GET_LOCAL, sizeof(index), buf);
}

void chunk_emit_set_local(Chunk* self, u16 index) {
    u8 buf[sizeof(index)];
    to_le_u16(index, buf);
    chunk_emit_args(self, OP_SET_LOCAL, sizeof(index), buf);
}

static void chunk_disassemble_simple(Chunk self, size_t* offset) {
    printf("%s", opcode_to_string(self.data[(*offset)++]));
}

static u8 chunk_get_u8(Chunk self, size_t offset) {
    return self.data[offset];
}

#define MAKE_CHUNK_DISASSEMBLE_LE(type, fmt) \
static void chunk_disassemble_le_ ## type(Chunk self, size_t* offset) { \
    type value = from_le_ ## type(self.data + *offset); \
    printf(fmt, value); \
    *offset += sizeof(value); \
}

MAKE_CHUNK_DISASSEMBLE_LE(u64, U64_FMT)
MAKE_CHUNK_DISASSEMBLE_LE(u32, U32_FMT)
MAKE_CHUNK_DISASSEMBLE_LE(u16, U16_FMT)

MAKE_CHUNK_DISASSEMBLE_LE(f64, "%f")
MAKE_CHUNK_DISASSEMBLE_LE(f32, "%f")

static void _chunk_disassemble_one(Chunk self, size_t* offset) {
    switch ((Opcode) self.data[*offset]) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_BIT_AND:
        case OP_BIT_XOR:
        case OP_BIT_OR:
        case OP_BIT_LSHIFT:
        case OP_BIT_RSHIFT:
        case OP_NEGATE:
        case OP_BIT_NOT:
        case OP_POP: {
            chunk_disassemble_simple(self, offset);
        } break;
        case OP_LOAD_INT: {
            chunk_disassemble_simple(self, offset);
            printf(" ");
            chunk_disassemble_le_u64(self, offset);
        } break;
        case OP_LOAD_FLOAT: {
            chunk_disassemble_simple(self, offset);
            printf(" ");
            chunk_disassemble_le_f64(self, offset);
        } break;
        case OP_LOAD_BOOL: {
            chunk_disassemble_simple(self, offset);
            printf(" %s", chunk_get_u8(self, (*offset)++) ? "true" : "false");
        } break;
        case OP_LOAD_CHAR: {
            chunk_disassemble_simple(self, offset);
            printf(" '%c'", chunk_get_u8(self, (*offset)++));
        } break;
        case OP_GET_LOCAL:
        case OP_SET_LOCAL: {
            chunk_disassemble_simple(self, offset);
            printf(" ");
            chunk_disassemble_le_u16(self, offset);
        } break;
    }

    printf("\n");
}

void chunk_disassemble_one(Chunk self, size_t offset) {
    _chunk_disassemble_one(self, &offset);
}

void chunk_disassemble(Chunk self) {
    size_t offset = 0;

    while (offset < self.length) {
        _chunk_disassemble_one(self, &offset);
    }
}
