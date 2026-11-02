#include "string_view.h"

#include <string.h>

StringView sv_from_cstr(const char* str) {
    return sv_from_str_length(str, strlen(str));
}

StringView sv_from_str_length(const char* str, size_t length) {
    return (StringView) {
        .data = str,
        .length = length,
    };
}

StringView sv_slice(StringView sv, size_t start, size_t length) {
    return (StringView) {
        .data = sv.data + start,
        .length = length,
    };
}
