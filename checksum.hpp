#pragma once

#include <cstdint>

#include "msg_header.hpp"

inline void Checksum(const char* b, uint32_t len, MsgHeader& header) {
  uint16_t checksum = header.Checksum();
  header.Checksum(0);

  uint32_t sum = checksum;
  for (uint32_t j = 0; j < len - 1; j += 2) {
    sum += *((uint16_t*)(&b[j]));
  }
  if ((len & 1) != 0) {
    sum += *((uint8_t*)(&b[len - 1]));
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum = (sum >> 16) + (sum & 0xFFFF);

  header.Checksum(uint16_t(~sum));
}
