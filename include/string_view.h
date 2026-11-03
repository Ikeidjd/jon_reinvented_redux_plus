#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    const char* data;
    size_t length;
} StringView;

// Inspired by Tsoding
#define SV_FMT "%.*s"
#define SV_ARG(sv) (int) (sv).length, (sv).data

#define sv_lit(lit) sv_new(lit, sizeof(lit) / sizeof((lit)[0]) - 1)

StringView sv_cstr(const char* str);

StringView sv_new(const char* str, size_t length);

StringView sv_slice(StringView sv, size_t start, size_t length);

StringView sv_slice_from(StringView sv, size_t start);

StringView sv_slice_until(StringView sv, size_t length);

StringView sv_first_line(StringView sv);

bool sv_to_int(StringView sv, int* out);
