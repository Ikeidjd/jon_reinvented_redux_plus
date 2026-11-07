#pragma once

#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define U8_FMT "%"PRIu8
#define U16_FMT "%"PRIu16
#define U32_FMT "%"PRIu32
#define U64_FMT "%"PRIu64

#define I8_FMT "%"PRIi8
#define I16_FMT "%"PRIi16
#define I32_FMT "%"PRIi32
#define I64_FMT "%"PRIi64

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;
