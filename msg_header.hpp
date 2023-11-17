#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "endian.hpp"

class MsgHeader {
 public:
  MsgHeader() = delete;
  MsgHeader(char *buffer, size_t buffer_length, size_t buffer_offset)
      : buffer_{buffer},
        buffer_length_{buffer_length},
        buffer_offset_{buffer_offset} {
    if (buffer_length < buffer_offset + EncodedLength()) {
      throw std::runtime_error("buffer to small for MsgHeader");
    }
  }
  ~MsgHeader() = default;

  // Copy.
  MsgHeader(const MsgHeader &) = default;
  MsgHeader &operator=(const MsgHeader &) = default;

  // Move.
  MsgHeader(MsgHeader &&) = default;
  MsgHeader &operator=(MsgHeader &&) = default;

  [[nodiscard]] static constexpr size_t EncodedLength() noexcept { return 13; }

  [[nodiscard]] const char *Buffer() const noexcept { return buffer_; }

  [[nodiscard]] size_t BufferLength() const noexcept { return buffer_length_; }

  [[nodiscard]] size_t BufferOffset() const noexcept { return buffer_offset_; }

  // MsgType: char, 1 byte, offset 0
  [[nodiscard]] static constexpr size_t MsgTypeEncodingLength() noexcept {
    return 1;
  }

  [[nodiscard]] static constexpr size_t MsgTypeEncodingOffset() noexcept {
    return 0;
  }

  [[nodiscard]] char MsgType() const noexcept {
    return *(buffer_ + buffer_offset_ + MsgTypeEncodingOffset());
  }

  MsgHeader &MsgType(const char val) noexcept {
    *(buffer_ + buffer_offset_ + MsgTypeEncodingOffset()) = val;
    return *this;
  }

  // MsgLen: u16, 2 bytes, offset 1
  [[nodiscard]] static constexpr size_t MsgLenEncodingLength() noexcept {
    return 2;
  }

  [[nodiscard]] static constexpr size_t MsgLenEncodingOffset() noexcept {
    return 1;
  }

  [[nodiscard]] uint16_t MsgLen() const noexcept {
    uint16_t val{};
    std::memcpy(&val, buffer_ + buffer_offset_ + MsgLenEncodingOffset(),
                sizeof(uint16_t));
    return LITTLE_ENDIAN_ENCODE_16(val);
  }

  MsgHeader &MsgLen(const uint16_t val) noexcept {
    uint16_t value = LITTLE_ENDIAN_ENCODE_16(val);
    std::memcpy(buffer_ + buffer_offset_ + MsgLenEncodingOffset(), &value,
                sizeof(uint16_t));
    return *this;
  }

  // Timestamp: u64, 8 bytes, offset 3
  [[nodiscard]] static constexpr size_t TimestampEncodingLength() noexcept {
    return 8;
  }

  [[nodiscard]] static constexpr size_t TimestampEncodingOffset() noexcept {
    return 3;
  }

  [[nodiscard]] uint64_t Timestamp() const noexcept {
    uint64_t val{};
    std::memcpy(&val, buffer_ + buffer_offset_ + TimestampEncodingOffset(),
                sizeof(uint64_t));
    return LITTLE_ENDIAN_ENCODE_64(val);
  }

  MsgHeader &Timestamp(const uint64_t val) noexcept {
    uint64_t value = LITTLE_ENDIAN_ENCODE_64(val);
    std::memcpy(buffer_ + buffer_offset_ + TimestampEncodingOffset(), &value,
                sizeof(uint64_t));
    return *this;
  }

  // ChkSum: u16, 2 bytes, offset 11
  [[nodiscard]] static constexpr size_t ChecksumEncodingLength() noexcept {
    return 2;
  }

  [[nodiscard]] static constexpr size_t ChecksumEncodingOffset() noexcept {
    return 11;
  }

  [[nodiscard]] uint16_t Checksum() const noexcept {
    uint16_t val{};
    std::memcpy(&val, buffer_ + buffer_offset_ + ChecksumEncodingOffset(),
                sizeof(uint16_t));
    return LITTLE_ENDIAN_ENCODE_16(val);
  }

  MsgHeader &Checksum(const uint16_t val) noexcept {
    uint16_t value = LITTLE_ENDIAN_ENCODE_16(val);
    std::memcpy(buffer_ + buffer_offset_ + ChecksumEncodingOffset(), &value,
                sizeof(uint16_t));
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const MsgHeader &header) {
    os << "header msg_type=" << header.MsgType()
       << " msg_len=" << header.MsgLen() << " timestamp=" << header.Timestamp()
       << " checksum=" << header.Checksum();
    return os;
  }

 private:
  char *buffer_;
  size_t buffer_length_;
  size_t buffer_offset_;
};
