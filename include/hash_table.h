#pragma once

#include "typedef_utils.h"

// Forward declaration to avoid circular include
typedef struct Type Type;

typedef u64 hash_t;
typedef hash_t (*HashFn)(Type* type);
typedef bool (*EqFn)(Type* a, Type* b);

typedef struct {
    Type* type;
    bool deleted;
} HashTableBucket;

typedef struct {
    HashTableBucket* data;
    size_t occupied;
    size_t capacity;
    HashFn hash_fn;
    EqFn eq_fn;
} HashTable_Type;

void hash_table_init(HashTable_Type* self, HashFn hash_fn, EqFn eq_fn);

void hash_table_destroy(HashTable_Type* self);

Type* hash_table_get(const HashTable_Type* self, Type* type);

void hash_table_insert(HashTable_Type* self, Type* type);

Type* hash_table_get_or_insert(HashTable_Type* self, Type* type);
