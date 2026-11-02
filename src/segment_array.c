#include "segment_array.h"

#include "dynamic_array.h"
#include "math_utils.h"

void sega_init(SegmentArray* sega) {
    *sega = (SegmentArray) {0};
    dyna_init(&sega->segments);
}

void sega_append(SegmentArray* sega, int i) {
    if (sega->segments.length == 0 || sega->length_of_active >= (size_t) (1 << (sega->segments.length - 1))) {
        sega->length_of_active = 0;

        SegaSegment segment = {
            .data = malloc((1 << sega->segments.length) * sizeof(i)),
            .length = 0,
        };

        dyna_append(&sega->segments, segment);
    }

    SegaSegment* segment = &sega->segments.data[sega->segments.length - 1];
    segment->data[segment->length++] = i;
    sega->length_of_active++;
}

void sega_get_indices(size_t index, size_t* index_of_segment, size_t* index_in_segment) {
    *index_of_segment = log2sz(index + 1);
    *index_in_segment = index + 1 - (1 << *index_of_segment);
}

int* sega_get_ptr(SegmentArray* sega, size_t index) {
    size_t index_of_segment, index_in_segment;
    sega_get_indices(index, &index_of_segment, &index_in_segment);
    return &sega->segments.data[index_of_segment].data[index_in_segment];
}

int sega_get(SegmentArray* sega, size_t index) {
    int* ptr = sega_get_ptr(sega, index);
    return *ptr;
}

void sega_set(SegmentArray* sega, size_t index, int i) {
    int* ptr = sega_get_ptr(sega, index);
    *ptr = i;
}
