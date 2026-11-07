#pragma once

#include "typedef_utils.h"

void to_le_u64(u64 integer, u8 buf[sizeof(integer)]);
void to_le_u32(u32 integer, u8 buf[sizeof(integer)]);
void to_le_u16(u16 integer, u8 buf[sizeof(integer)]);

void to_le_f64(f64 integer, u8 buf[sizeof(integer)]);
void to_le_f32(f32 integer, u8 buf[sizeof(integer)]);

u64 from_le_u64(u8 buf[8]);
u32 from_le_u32(u8 buf[4]);
u16 from_le_u16(u8 buf[2]);

f64 from_le_f64(u8 buf[8]);
f32 from_le_f32(u8 buf[4]);
