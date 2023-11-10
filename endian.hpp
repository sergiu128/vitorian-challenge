#pragma once

#include <byteswap.h>
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define LITTLE_ENDIAN_ENCODE_16(v) (v)
#define LITTLE_ENDIAN_ENCODE_32(v) (v)
#define LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define LITTLE_ENDIAN_ENCODE_16(v) __bswap_16(v)
#define LITTLE_ENDIAN_ENCODE_32(v) __bswap_32(v)
#define LITTLE_ENDIAN_ENCODE_64(v) __bswap_64(v)
#endif
