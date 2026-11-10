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
    type integer = from_le_ ## type(chunk.bytes.data + *ip); \
    *ip += sizeof(integer); \
    return integer; \
}

MAKE_READ(u64)
MAKE_READ(u32)
MAKE_READ(u16)

MAKE_READ(f64)
MAKE_READ(f32)

static void vm_execute_chunk(VM* self, Chunk chunk) {
    #define ARITH_OP(op) \
    do { \
        Value b = vm_pop(self); \
        Value a = vm_pop(self); \
        vm_push(self, value_ ## op(a, b)); \
    } while (0)

    #define COMP_OP(op) \
    do { \
        Value b = vm_pop(self); \
        Value a = vm_pop(self); \
        vm_push(self, (Value) { \
            .type = VALUE_BOOL, \
            .as.boolean = value_ ## op(a, b), \
        }); \
    } while (0)

    #define UNARY_OP(op) \
    do { \
        Value value = vm_pop(self); \
        vm_push(self, value_ ## op(value)); \
    } while (0)

    #define READ_U64() _read_u64(chunk, &ip)
    #define READ_U32() _read_u32(chunk, &ip)
    #define READ_U16() _read_u16(chunk, &ip)
    #define READ_U8() chunk.bytes.data[ip++]

    #define READ_F64() _read_f64(chunk, &ip)
    #define READ_F32() _read_f32(chunk, &ip)

    #define READ_OPCODE() ((Opcode) READ_U8())

    size_t ip = 0;

    while (ip < chunk.bytes.length) {
        vm_print_stack(self);
        chunk_disassemble_one(chunk, ip);

        switch (READ_OPCODE()) {
            case OP_ADD: {
                ARITH_OP(add);
            } break;
            case OP_SUB: {
                ARITH_OP(sub);
            } break;
            case OP_MUL: {
                ARITH_OP(mul);
            } break;
            case OP_DIV: {
                ARITH_OP(div);
            } break;
            case OP_MOD: {
                ARITH_OP(mod);
            } break;
            case OP_BIT_AND: {
                ARITH_OP(bit_and);
            } break;
            case OP_BIT_XOR: {
                ARITH_OP(bit_xor);
            } break;
            case OP_BIT_OR: {
                ARITH_OP(bit_or);
            } break;
            case OP_BIT_LSHIFT: {
                ARITH_OP(bit_lshift);
            } break;
            case OP_BIT_RSHIFT: {
                ARITH_OP(bit_rshift);
            } break;
            case OP_EQ: {
                COMP_OP(eq);
            } break;
            case OP_NE: {
                COMP_OP(ne);
            } break;
            case OP_LT: {
                COMP_OP(lt);
            } break;
            case OP_LE: {
                COMP_OP(le);
            } break;
            case OP_GT: {
                COMP_OP(gt);
            } break;
            case OP_GE: {
                COMP_OP(ge);
            } break;
            case OP_NEGATE: {
                UNARY_OP(negate);
            } break;
            case OP_BIT_NOT: {
                UNARY_OP(bit_not);
            } break;
            case OP_NOT: {
                UNARY_OP(not);
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
            case OP_DUP: {
                vm_dup(self);
            } break;
            case OP_JUMP: {
                ip += READ_U16() - 3;
            } break;
            case OP_JUMP_IF_TRUE: {
                u16 increment = READ_U16() - 3;
                if (vm_pop(self).as.boolean) ip += increment;
            } break;
            case OP_JUMP_IF_FALSE: {
                u16 increment = READ_U16() - 3;
                if (!vm_pop(self).as.boolean) ip += increment;
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
    #undef COMP_OP
    #undef ARITH_OP
}

static void vm_push(VM* self, Value value) {
    self->stack[self->sp++] = value;
}

static Value vm_pop(VM* self) {
    return self->stack[--self->sp];
}

static void vm_dup(VM* self) {
    vm_push(self, vm_top(self));
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
