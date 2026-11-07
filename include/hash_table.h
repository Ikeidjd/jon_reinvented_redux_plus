#pragma once

#include <stdbool.h>
#include <stdint.h>

// Forward declaration to avoid circular include
typedef struct Type Type;

typedef uint64_t hash_t;
typedef hash_t (*HashFn)(Type* type);
typedef bool (*EqFn)(Type* a, Type* b);

typedef struct {
    Type* type;
    bool deleted;
} HashTableBucket;

typedef struct {
    HashTableBucket* data;
    size_t length;
    size_t capacity;
    HashFn hash_fn;
    EqFn eq_fn;
} HashTable_Type;

void hash_table_init(HashTable_Type* self, HashFn hash_fn, EqFn eq_fn);

Type* hash_table_get(const HashTable_Type* self, Type* type);

void hash_table_insert(HashTable_Type* self, Type* type);

Type* hash_table_get_or_insert(HashTable_Type* self, Type* type);
