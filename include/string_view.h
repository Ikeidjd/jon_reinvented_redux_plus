#pragma once

#include "typedef_utils.h"

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

StringView sv_trim_left(StringView sv);

StringView sv_trim_right(StringView sv);

StringView sv_trim(StringView sv);

bool sv_eq(StringView a, StringView b);

bool sv_eq_cstr(StringView sv, const char* cstr);

bool sv_to_i64(StringView sv, i64* out);

bool sv_hex_to_i64(StringView sv, i64* out);

bool sv_oct_to_i64(StringView sv, i64* out);

bool sv_bin_to_i64(StringView sv, i64* out);

bool sv_to_f64(StringView sv, f64* out);
