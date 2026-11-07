#include "little_endian_utils.h"

#define MAKE_TO_LE_UINT(bits) \
void to_le_u ## bits(u ## bits integer, u8 buf[sizeof(integer)]) { \
    for (size_t i = 0; i < sizeof(integer); i++) { \
        buf[i] = (integer >> (i * 8)) & 0xFF; \
    } \
}

#define MAKE_TO_LE_FLOAT(bits) \
void to_le_f ## bits(f ## bits f, u8 buf[sizeof(f)]) { \
    to_le_u ## bits(*(u ## bits*) &f, buf); \
}

MAKE_TO_LE_UINT(64)
MAKE_TO_LE_UINT(32)
MAKE_TO_LE_UINT(16)

MAKE_TO_LE_FLOAT(64)
MAKE_TO_LE_FLOAT(32)

// It's very important to cast to the corresponding type before bitshifting to avoid weird wrap-arounds with u64 (this threw me off for a while)
#define MAKE_FROM_LE_UINT(bits) \
u ## bits from_le_u ## bits(u8 buf[bits / 8]) { \
    u ## bits integer = 0; \
    for (size_t i = 0; i < sizeof(integer); i++) { \
        integer |= (u ## bits) buf[i] << (i * 8); \
    } \
    return integer; \
}

#define MAKE_FROM_LE_FLOAT(bits) \
f ## bits from_le_f ## bits(u8 buf[bits / 8]) { \
    u ## bits integer = from_le_u ## bits(buf); \
    return *(f ## bits*) &integer; \
}

MAKE_FROM_LE_UINT(64)
MAKE_FROM_LE_UINT(32)
MAKE_FROM_LE_UINT(16)

MAKE_FROM_LE_FLOAT(64)
MAKE_FROM_LE_FLOAT(32)
