#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
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

  [[nodiscard]] char MsgType() const noexcept { return 'E'; }

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

  [[nodiscard]] const char *Reason() const noexcept {
    return buffer_ + buffer_offset_ + ReasonEncodingOffset();
  }

  template <typename ClaimFnT>
  LoginResponse &Reason(ClaimFnT &&fn) noexcept {
    char *b = buffer_ + buffer_offset_ + ReasonEncodingOffset();
    size_t max_length = ReasonEncodingLength() - 1;

    size_t n_used = fn(b, max_length);
    if (n_used > max_length) {
      n_used = max_length;
    }
    *(b + n_used) = '\0';

    return *this;
  }

 private:
  char *buffer_;
  size_t buffer_length_;
  size_t buffer_offset_;
};
