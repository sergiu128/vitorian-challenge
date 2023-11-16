#pragma once

#include <cstdint>

#include "endian.hpp"
#include "msg_header.hpp"

inline void Checksum(const char* b, uint32_t len, MsgHeader& header) {
  uint16_t checksum = header.Checksum();
  header.Checksum(0);

  uint32_t sum = checksum;
  for (uint32_t j = 0; j < len - 1; j += 2) {
    uint16_t value = *((uint16_t*)(&b[j]));
    sum += LITTLE_ENDIAN_ENCODE_16(value);
  }
  if ((len & 1) != 0) {
    sum += *((uint8_t*)(&b[len - 1]));
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum = (sum >> 16) + (sum & 0xFFFF);

  header.Checksum(uint16_t(~sum));
}
