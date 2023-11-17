#pragma once

#include <cstdint>

#include "msg_header.hpp"

// Compute the IP checksum of the bytes in b of length len that the header is
// also part of.
//
// If computing the checksum, callers must ensure the checksum is 0 in the
// passed MsgHeader before the call. The checksum of the rest of the bytes is
// then places in the header.
//
// If verifying the checksum, callers must ensure to have some checksum set in
// the header before this call. If the checksum is valid and the verification
// succeeds, the header will contain 0 in its checksum field after this call.
inline void Checksum(const char* b, uint32_t len, MsgHeader& header) {
  uint16_t checksum = header.Checksum();  // get the current checksum (can be 0)
  header.Checksum(0);                     // set checksum to 0

  uint32_t sum = checksum;
  for (uint32_t j = 0; j < len - 1; j += 2) {
    sum += *((uint16_t*)(&b[j]));
  }
  if ((len & 1) != 0) {
    sum += *((uint8_t*)(&b[len - 1]));
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum = (sum >> 16) + (sum & 0xFFFF);

  header.Checksum(uint16_t(~sum));  // place the new checksum in the header
}
