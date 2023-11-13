#pragma once

#include <cstdint>

inline uint16_t Checksum(const char* b, uint32_t len) {
  uint32_t sum = 0;
  for (uint32_t j = 0; j < len - 1; j += 2) {
    sum += *((uint16_t*)(&b[j]));
  }
  if ((len & 1) != 0) {
    sum += b[len - 1];
  }
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum = (sum >> 16) + (sum & 0xFFFF);
  return uint16_t(~sum);
}
