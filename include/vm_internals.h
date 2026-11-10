#pragma once

#include "value.h"

#define STACK_CAPACITY 65536

typedef struct {
    Value stack[STACK_CAPACITY];
    u16 sp;
} VM;

static void vm_init(VM* self);

static void vm_execute_chunk(VM* self, Chunk chunk);

static void vm_push(VM* self, Value value);

static Value vm_pop(VM* self);

static void vm_popn(VM* self, u16 n);

static void vm_dup(VM* self);

static Value vm_top(VM* self);

static void vm_print_stack(VM* self);
