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

static void hash_table_set_no_resize(HashTable_Type* self, HashTableBucket* bucket, Type* type) {
    bucket->type = type;
    self->length++;
}

static void hash_table_resize(HashTable_Type* self) {
    HashTableBucket* old_data = self->data;
    size_t old_capacity = self->capacity;

    self->capacity *= 2;
    self->data = malloc(self->capacity);

    for (size_t i = 0; i < old_capacity; i++) {
        if (!old_data[i].deleted && old_data[i].type != NULL) {
            HashTableBucket* bucket = hash_table_get_bucket(self, old_data[i].type);
            hash_table_set_no_resize(self, bucket, old_data[i].type);
        }
    }

    free(old_data);
}

static void hash_table_maybe_resize(HashTable_Type* self) {
    if (self->length * 4 >= self->capacity * 3) {
        hash_table_resize(self);
    }
}

static void hash_table_set_maybe_resize(HashTable_Type* self, HashTableBucket* bucket, Type* type) {
    hash_table_set_no_resize(self, bucket, type);
    hash_table_maybe_resize(self);
}

void hash_table_insert(HashTable_Type* self, Type* type) {
    HashTableBucket* bucket = hash_table_get_bucket(self, type);
    hash_table_set_maybe_resize(self, bucket, type);
}

Type* hash_table_get_or_insert(HashTable_Type* self, Type* type) {
    HashTableBucket* bucket = hash_table_get_bucket(self, type);
    type = bucket->type;

    if (bucket->type == NULL) {
        // A resize might happen here
        hash_table_set_maybe_resize(self, bucket, type);
    }

    // You can't return bucket->type because a resize might happen right before this, invalidating the pointer
    return type;
}
