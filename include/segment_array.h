#pragma once

#include <stddef.h>

#include "dynamic_array.h"

// Inspired by https://danielchasehooper.com/posts/segment_array/
typedef struct {
    void* data;
    size_t length;
} SegaSegment;

typedef struct {
    DynamicArray(SegaSegment) segments;
    size_t length_of_active;
    size_t total_length;
} _SegmentArray;

#define SegmentArray(type) \
struct { \
    union { \
        _SegmentArray _sega; \
        type* _type_info; \
    }; \
}

// Sega? Holy Sonic reference
void _sega_init(_SegmentArray* sega, size_t elem_size);

void _sega_maybe_resize(_SegmentArray* sega, size_t elem_size);

void* _sega_get(_SegmentArray* sega, size_t index, size_t elem_size);

void sega_get_indices(size_t index, size_t* index_of_segment, size_t* index_in_segment);

void _sega_destroy(_SegmentArray* sega);

#define sega_init(sega) _sega_init(&(sega)->_sega, sizeof(*(sega)->_type_info))

// OH MY GOD!!!!
// I forgot the asterisk in sizeof(*(sega)->_type_info), meaning that when using Sega_Node, the size was 8 instead of 56
// No wonder it segfaulted...
// Thanks to Dr. Memory for making this easier to spot
#define sega_append(sega, value) \
do { \
    _SegmentArray* real = &(sega)->_sega; \
    _sega_maybe_resize(real, sizeof(*(sega)->_type_info)); \
    sega_set(sega, real->total_length, value); \
\
    real->segments.data[real->segments.length - 1].length++; \
    real->length_of_active++; \
    real->total_length++; \
} while (0)

#define sega_get_ptr(sega, index) ((typeof((sega)->_type_info)) _sega_get(&(sega)->_sega, index, sizeof(*(sega)->_type_info)))

#define sega_get(sega, index) *sega_get_ptr(sega, index)

#define sega_set(sega, index, value) *sega_get_ptr(sega, index) = value

#define sega_length(sega) (sega)._sega.total_length

#define sega_destroy(sega) _sega_destroy(&(sega)._sega)
