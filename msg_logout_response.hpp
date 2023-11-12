#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
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

  [[nodiscard]] const char *Reason() const noexcept {
    return buffer_ + buffer_offset_ + ReasonEncodingOffset();
  }

  template <typename ClaimFnT>
  LogoutResponse &Reason(ClaimFnT &&fn) noexcept {
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
