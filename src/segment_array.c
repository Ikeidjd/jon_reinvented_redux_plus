#include "segment_array.h"

#include <stdlib.h>

#include "dynamic_array.h"
#include "math_utils.h"

void _sega_maybe_resize(_SegmentArray* sega, size_t elem_size) {
    if (sega->segments.length == 0 || sega->length_of_active >= (1ull << (sega->segments.length - 1))) {
        SegaSegment segment = {
            .data = malloc((1ull << sega->segments.length) * elem_size),
            .length = 0,
        };

        dyna_append(&sega->segments, segment);
        sega->length_of_active = 0;
    }
}

void* _sega_get(_SegmentArray* sega, size_t index, size_t elem_size) {
    size_t index_of_segment, index_in_segment;
    sega_get_indices(index, &index_of_segment, &index_in_segment);

    index_in_segment *= elem_size;
    return (void*) ((char*) sega->segments.data[index_of_segment].data + index_in_segment);
}

void _sega_init(_SegmentArray* sega, size_t elem_size) {
    *sega = (_SegmentArray) {0};
    dyna_init_with_capacity(&sega->segments, DYNA_INITIAL_CAPACITY * elem_size);
}

void sega_get_indices(size_t index, size_t* index_of_segment, size_t* index_in_segment) {
    *index_of_segment = log2sz(index + 1);
    *index_in_segment = index + 1 - (1 << *index_of_segment);
}

void _sega_destroy(_SegmentArray* sega) {
    for (size_t i = 0; i < sega->segments.length; i++) {
        free(sega->segments.data[i].data);
    }

    dyna_destroy(&sega->segments);
}
