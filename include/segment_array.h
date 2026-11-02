#pragma once

#include <stddef.h>

// Inspired by https://danielchasehooper.com/posts/segment_array/
typedef struct {
    int* data;
    size_t length;
} SegaSegment;

typedef struct {
    SegaSegment* data;
    size_t capacity;
    size_t length;
} SegaSegments;

typedef struct {
    SegaSegments segments;
    size_t length_of_active;
} SegmentArray;

// Sega? Holy Sonic reference
void sega_init(SegmentArray* sega);

void sega_append(SegmentArray* sega, int i);

void sega_get_indices(size_t index, size_t* index_of_segment, size_t* index_in_segment);

int* sega_get_ptr(SegmentArray* sega, size_t index);

int sega_get(SegmentArray* sega, size_t index);

void sega_set(SegmentArray* sega, size_t index, int i);
