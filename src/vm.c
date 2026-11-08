#include "vm.h"
#include "vm_internals.h"

#include "little_endian_utils.h"

void execute_chunk(Chunk chunk) {
    VM* self = malloc(sizeof(*self));
    vm_init(self);

    vm_execute_chunk(self, chunk);
    free(self);
}

static void vm_init(VM* self) {
    self->sp = 0;
}

#define MAKE_READ(type) \
static type _read_ ## type(Chunk chunk, size_t* ip) { \
    type integer = from_le_ ## type(chunk.data + *ip); \
    *ip += sizeof(integer); \
    return integer; \
}

MAKE_READ(u64)
MAKE_READ(u32)
MAKE_READ(u16)

MAKE_READ(f64)
MAKE_READ(f32)

static void vm_execute_chunk(VM* self, Chunk chunk) {
    #define BIN_OP(op) \
    do { \
        Value b = vm_pop(self); \
        Value a = vm_pop(self); \
        vm_push(self, value_ ## op(a, b)); \
    } while (0)

    #define UNARY_OP(op) \
    do { \
        Value value = vm_pop(self); \
        vm_push(self, value_ ## op(value)); \
    } while (0)

    #define READ_U64() _read_u64(chunk, &ip)
    #define READ_U32() _read_u32(chunk, &ip)
    #define READ_U16() _read_u16(chunk, &ip)
    #define READ_U8() chunk.data[ip++]

    #define READ_F64() _read_f64(chunk, &ip)
    #define READ_F32() _read_f32(chunk, &ip)

    #define READ_OPCODE() ((Opcode) READ_U8())

    size_t ip = 0;

    while (ip < chunk.length) {
        vm_print_stack(self);
        chunk_disassemble_one(chunk, ip);

        switch (READ_OPCODE()) {
            case OP_ADD: {
                BIN_OP(add);
            } break;
            case OP_SUB: {
                BIN_OP(sub);
            } break;
            case OP_MUL: {
                BIN_OP(mul);
            } break;
            case OP_DIV: {
                BIN_OP(div);
            } break;
            case OP_MOD: {
                BIN_OP(mod);
            } break;
            case OP_NEGATE: {
                UNARY_OP(negate);
            } break;
            case OP_LOAD_INT: {
                vm_push(self, (Value) {
                    .type = VALUE_INT,
                    .as.integer = (i64) READ_U64(),
                });
            } break;
            case OP_LOAD_FLOAT: {
                vm_push(self, (Value) {
                    .type = VALUE_FLOAT,
                    .as.floating = READ_F64(),
                });
            } break;
            case OP_LOAD_BOOL: {
                vm_push(self, (Value) {
                    .type = VALUE_BOOL,
                    .as.boolean = (bool) READ_U8(),
                });
            } break;
            case OP_LOAD_CHAR: {
                vm_push(self, (Value) {
                    .type = VALUE_CHAR,
                    .as.integer = (char) READ_U8(),
                });
            } break;
            case OP_GET_LOCAL: {
                vm_push(self, self->stack[READ_U16()]);
            } break;
            case OP_SET_LOCAL: {
                self->stack[READ_U16()] = vm_top(self);
            } break;
            case OP_POP: {
                vm_pop(self);
            } break;
        }
    }

    vm_print_stack(self);

    #undef READ_OPCODE

    #undef READ_F32
    #undef READ_F64

    #undef READ_U8
    #undef READ_U16
    #undef READ_U32
    #undef READ_U64

    #undef UNARY_OP
    #undef BIN_OP
}

static void vm_push(VM* self, Value value) {
    self->stack[self->sp++] = value;
}

static Value vm_pop(VM* self) {
    return self->stack[--self->sp];
}

static Value vm_top(VM* self) {
    return self->stack[self->sp - 1];
}

static void vm_print_stack(VM* self) {
    if (self->sp == 0) printf("<empty stack>");

    for (u16 i = 0; i < self->sp; i++) {
        printf("[");
        value_repr_print(self->stack[i]);
        printf("]");
    }

    printf("\n");
}
