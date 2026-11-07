#include "hash_table.h"

#include <stdlib.h>
#include <string.h>

// IMPORTANT: make sure this is a power of two
// Modulo operations can be converted to bitwise operations if you do
#define HASH_TABLE_INITIAL_CAPACITY 128

void hash_table_init(HashTable_Type* self, HashFn hash_fn, EqFn eq_fn) {
    *self = (HashTable_Type) {
        .data = malloc(HASH_TABLE_INITIAL_CAPACITY),
        .length = 0,
        .capacity = HASH_TABLE_INITIAL_CAPACITY,
        .hash_fn = hash_fn,
        .eq_fn = eq_fn,
    };

    memset(self->data, 0, self->capacity);
}

static HashTableBucket* hash_table_get_bucket(const HashTable_Type* self, Type* type) {
    hash_t hash = self->hash_fn(type);
    hash_t index = hash & (self->capacity - 1);

    for (;; index = (index + 1) & (self->capacity - 1)) {
        if (!self->data[index].deleted && (self->data[index].type == NULL || self->eq_fn(type, self->data[index].type))) return &self->data[index];
    }
}

Type* hash_table_get(const HashTable_Type* self, Type* type) {
    HashTableBucket* bucket = hash_table_get_bucket(self, type);
    return bucket->type;
}

void hash_table_insert(HashTable_Type* self, Type* type) {
    HashTableBucket* bucket = hash_table_get_bucket(self, type);
    bucket->type = type;
}

Type* hash_table_get_or_insert(HashTable_Type* self, Type* type) {
    HashTableBucket* bucket = hash_table_get_bucket(self, type);
    if (bucket->type == NULL) bucket->type = type;
    return bucket->type;
}
