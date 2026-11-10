#include "bytecode.h"

#include <stdio.h>
#include <assert.h>

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
        case OP_EQ:
            return "OP_EQ";
        case OP_NE:
            return "OP_NE";
        case OP_LT:
            return "OP_LT";
        case OP_LE:
            return "OP_LE";
        case OP_GT:
            return "OP_GT";
        case OP_GE:
            return "OP_GE";
        case OP_NEGATE:
            return "OP_NEGATE";
        case OP_BIT_NOT:
            return "OP_BIT_NOT";
        case OP_NOT:
            return "OP_NOT";
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
        case OP_DUP:
            return "OP_DUP";
        case OP_JUMP:
            return "OP_JUMP";
        case OP_JUMP_IF_TRUE:
            return "OP_JUMP_IF_TRUE";
        case OP_JUMP_IF_FALSE:
            return "OP_JUMP_IF_FALSE";
    }

    return "";
}

void chunk_init(Chunk* self) {
    dyna_init(&self->bytes);
    dyna_init(&self->lines);
}

void chunk_destroy(Chunk* self) {
    dyna_destroy(&self->bytes);
    dyna_destroy(&self->lines);
}

void chunk_emit(Chunk* self, Opcode opcode, size_t line) {
    dyna_append(&self->bytes, opcode);
    dyna_append(&self->lines, line);
}

void chunk_emit_args(Chunk* self, Opcode opcode, size_t n, u8 args[n], size_t line) {
    chunk_emit(self, opcode, line);

    for (size_t i = 0; i < n; i++) {
        chunk_emit(self, args[i], line);
    }
}

void chunk_emit_load_int(Chunk* self, i64 i, size_t line) {
    u8 buf[sizeof(i)];
    to_le_u64((u64) i, buf);
    chunk_emit_args(self, OP_LOAD_INT, sizeof(i), buf, line);
}

void chunk_emit_load_float(Chunk* self, f64 f, size_t line) {
    u8 buf[sizeof(f)];
    to_le_f64(f, buf);
    chunk_emit_args(self, OP_LOAD_FLOAT, sizeof(f), buf, line);
}

void chunk_emit_load_bool(Chunk* self, bool b, size_t line) {
    chunk_emit_args(self, OP_LOAD_BOOL, sizeof(b), (u8*) &b, line);
}

void chunk_emit_load_char(Chunk* self, char c, size_t line) {
    chunk_emit_args(self, OP_LOAD_CHAR, sizeof(c), (u8*) &c, line);
}

void chunk_emit_get_local(Chunk* self, u16 index, size_t line) {
    u8 buf[sizeof(index)];
    to_le_u16(index, buf);
    chunk_emit_args(self, OP_GET_LOCAL, sizeof(index), buf, line);
}

void chunk_emit_set_local(Chunk* self, u16 index, size_t line) {
    u8 buf[sizeof(index)];
    to_le_u16(index, buf);
    chunk_emit_args(self, OP_SET_LOCAL, sizeof(index), buf, line);
}

size_t chunk_emit_jump(Chunk* self, Opcode opcode, size_t line) {
    size_t jump_op_index = self->bytes.length;
    chunk_emit_args(self, opcode, 2, (u8[2]) {0, 0}, line);
    return jump_op_index;
}

void chunk_patch_jump(Chunk* self, size_t index) {
    size_t increment = self->bytes.length - index;

    if ((u16) increment != increment) {
        assert(false && "Jump too far");
    }

    to_le_u16((u16) increment, self->bytes.data + index + 1);
}

static void chunk_disassemble_simple(Chunk self, size_t* offset) {
    size_t line = self.lines.data[*offset];
    Opcode opcode = self.bytes.data[*offset];

    printf("LINE %04zu: %s", line + 1, opcode_to_string(opcode));
    *offset += 1;
}

static u8 chunk_get_u8(Chunk self, size_t offset) {
    return self.bytes.data[offset];
}

#define MAKE_CHUNK_DISASSEMBLE_LE(type, fmt) \
static void chunk_disassemble_le_ ## type(Chunk self, size_t* offset) { \
    type value = from_le_ ## type(self.bytes.data + *offset); \
    printf(fmt, value); \
    *offset += sizeof(value); \
}

MAKE_CHUNK_DISASSEMBLE_LE(u64, U64_FMT)
MAKE_CHUNK_DISASSEMBLE_LE(u32, U32_FMT)
MAKE_CHUNK_DISASSEMBLE_LE(u16, U16_FMT)

MAKE_CHUNK_DISASSEMBLE_LE(f64, "%f")
MAKE_CHUNK_DISASSEMBLE_LE(f32, "%f")

static void _chunk_disassemble_one(Chunk self, size_t* offset) {
    printf("BYTE %03zu: ", *offset);

    switch ((Opcode) self.bytes.data[*offset]) {
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
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        case OP_NEGATE:
        case OP_BIT_NOT:
        case OP_NOT:
        case OP_POP:
        case OP_DUP: {
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
        case OP_SET_LOCAL:
        case OP_JUMP:
        case OP_JUMP_IF_TRUE:
        case OP_JUMP_IF_FALSE: {
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

    while (offset < self.bytes.length) {
        _chunk_disassemble_one(self, &offset);
    }
}
