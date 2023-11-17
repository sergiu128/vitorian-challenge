#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

class LogoutResponse {
 public:
  LogoutResponse() = delete;
  LogoutResponse(char *buffer, size_t buffer_length, size_t buffer_offset)
      : buffer_{buffer},
        buffer_length_{buffer_length},
        buffer_offset_{buffer_offset} {
    if (buffer_length < buffer_offset + EncodedLength()) {
      throw std::runtime_error("buffer to small for LogoutResponse");
    }
  }
  ~LogoutResponse() = default;

  // Copy.
  LogoutResponse(const LogoutResponse &) = default;
  LogoutResponse &operator=(const LogoutResponse &) = default;

  // Move.
  LogoutResponse(LogoutResponse &&) = default;
  LogoutResponse &operator=(LogoutResponse &&) = default;

  [[nodiscard]] static constexpr char MsgType() noexcept { return 'G'; }

  [[nodiscard]] static constexpr size_t EncodedLength() noexcept { return 32; }

  [[nodiscard]] const char *Buffer() const noexcept { return buffer_; }
  [[nodiscard]] size_t BufferLength() const noexcept { return buffer_length_; }
  [[nodiscard]] size_t BufferOffset() const noexcept { return buffer_offset_; }

  // Reason: text, 32 bytes, offset 0
  [[nodiscard]] static constexpr size_t ReasonEncodingLength() noexcept {
    return 32;
  }

  [[nodiscard]] static constexpr size_t ReasonEncodingOffset() noexcept {
    return 0;
  }

  [[nodiscard]] std::string Reason() const noexcept {
    const char *b = buffer_ + buffer_offset_ + ReasonEncodingOffset();
    return std::string{b};
  }

  LogoutResponse &Reason(const std::string &val) noexcept {
    char *b = buffer_ + buffer_offset_ + ReasonEncodingOffset();
    size_t len = std::min(val.size(), ReasonEncodingLength());

    memcpy(b, val.c_str(), len);
    memset(b + len, 0, ReasonEncodingLength() - len);

    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const LogoutResponse &res) {
    std::cout << "logout_response reason=" << res.Reason();
    return os;
  }

 private:
  char *buffer_;
  size_t buffer_length_;
  size_t buffer_offset_;
};
