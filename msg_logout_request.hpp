#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

class LogoutRequest {
 public:
  LogoutRequest() = delete;
  LogoutRequest(char *buffer, size_t buffer_length, size_t buffer_offset)
      : buffer_{buffer},
        buffer_length_{buffer_length},
        buffer_offset_{buffer_offset} {
    if (buffer_length < buffer_offset + EncodedLength()) {
      throw std::runtime_error("buffer to small for LogoutRequest");
    }
  }
  ~LogoutRequest() = default;

  // Copy.
  LogoutRequest(const LogoutRequest &) = default;
  LogoutRequest &operator=(const LogoutRequest &) = default;

  // Move.
  LogoutRequest(LogoutRequest &&) = default;
  LogoutRequest &operator=(LogoutRequest &&) = default;

  [[nodiscard]] static constexpr char MsgType() noexcept { return 'O'; }

  [[nodiscard]] static constexpr size_t EncodedLength() noexcept { return 0; }

  [[nodiscard]] const char *Buffer() const noexcept { return buffer_; }
  [[nodiscard]] size_t BufferLength() const noexcept { return buffer_length_; }
  [[nodiscard]] size_t BufferOffset() const noexcept { return buffer_offset_; }

 private:
  char *buffer_;
  size_t buffer_length_;
  size_t buffer_offset_;
};
