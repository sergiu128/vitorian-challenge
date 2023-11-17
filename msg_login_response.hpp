#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

class LoginResponse {
 public:
  LoginResponse() = delete;

  LoginResponse(char *buffer, size_t buffer_length, size_t buffer_offset)
      : buffer_{buffer},
        buffer_length_{buffer_length},
        buffer_offset_{buffer_offset} {
    if (buffer_length < buffer_offset + EncodedLength()) {
      throw std::runtime_error("buffer to small for LoginResponse");
    }
  }
  ~LoginResponse() = default;

  // Copy.
  LoginResponse(const LoginResponse &) = default;
  LoginResponse &operator=(const LoginResponse &) = default;

  // Move.
  LoginResponse(LoginResponse &&) = default;
  LoginResponse &operator=(LoginResponse &&) = default;

  [[nodiscard]] static constexpr char MsgType() noexcept { return 'E'; }

  [[nodiscard]] static constexpr size_t EncodedLength() noexcept { return 33; }

  [[nodiscard]] const char *Buffer() const noexcept { return buffer_; }
  [[nodiscard]] size_t BufferLength() const noexcept { return buffer_length_; }
  [[nodiscard]] size_t BufferOffset() const noexcept { return buffer_offset_; }

  // Code: char, 1 byte; 0 offset
  [[nodiscard]] static constexpr size_t CodeEncodingLength() noexcept {
    return 1;
  }

  [[nodiscard]] static constexpr size_t CodeEncodingOffset() noexcept {
    return 0;
  }

  [[nodiscard]] char Code() const noexcept {
    return *(buffer_ + buffer_offset_ + CodeEncodingOffset());
  }

  LoginResponse Code(char val) noexcept {
    *(buffer_ + buffer_offset_ + CodeEncodingOffset()) = val;
    return *this;
  }

  // Reason: text, 32 bytes; 1 offset
  [[nodiscard]] static constexpr size_t ReasonEncodingLength() noexcept {
    return 32;
  }

  [[nodiscard]] static constexpr size_t ReasonEncodingOffset() noexcept {
    return 1;
  }

  [[nodiscard]] std::string Reason() const noexcept {
    const char *b = buffer_ + buffer_offset_ + ReasonEncodingOffset();
    return std::string{b};
  }

  LoginResponse &Reason(const std::string &val) noexcept {
    char *b = buffer_ + buffer_offset_ + ReasonEncodingOffset();
    size_t len = std::min(val.size(), ReasonEncodingLength());

    memcpy(b, val.c_str(), len);
    memset(b + len, 0, ReasonEncodingLength() - len);

    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const LoginResponse &res) {
    os << "login_response code=" << res.Code() << " reason=" << res.Reason();
    return os;
  }

 private:
  char *buffer_;
  size_t buffer_length_;
  size_t buffer_offset_;
};
