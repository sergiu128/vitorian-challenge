#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

class LoginRequest {
 public:
  LoginRequest() = delete;
  LoginRequest(char *buffer, size_t buffer_length, size_t buffer_offset)
      : buffer_{buffer},
        buffer_length_{buffer_length},
        buffer_offset_{buffer_offset} {
    if (buffer_length < buffer_offset + EncodedLength()) {
      throw std::runtime_error("buffer to small for LoginRequest");
    }
  }
  ~LoginRequest() = default;

  // Copy.
  LoginRequest(const LoginRequest &) = default;
  LoginRequest &operator=(const LoginRequest &) = default;

  // Move.
  LoginRequest(LoginRequest &&) = default;
  LoginRequest &operator=(LoginRequest &&) = default;

  [[nodiscard]] static constexpr char MsgType() noexcept { return 'L'; }

  [[nodiscard]] static constexpr size_t EncodedLength() noexcept { return 96; }

  [[nodiscard]] const char *Buffer() const noexcept { return buffer_; }
  [[nodiscard]] size_t BufferLength() const noexcept { return buffer_length_; }
  [[nodiscard]] size_t BufferOffset() const noexcept { return buffer_offset_; }

  // User: text, 64 bytes, offset 0
  [[nodiscard]] static constexpr size_t UserEncodingLength() noexcept {
    return 64;
  }

  [[nodiscard]] static constexpr size_t UserEncodingOffset() noexcept {
    return 0;
  }

  [[nodiscard]] std::string User() const noexcept {
    const char *b = buffer_ + buffer_offset_ + UserEncodingOffset();
    return std::string{b};
  }

  LoginRequest &User(const std::string &val) noexcept {
    char *b = buffer_ + buffer_offset_ + UserEncodingOffset();
    size_t max_length = UserEncodingLength() - 1;
    size_t len = val.size();
    if (len > max_length) len = max_length;

    memcpy(b, val.c_str(), len);
    *(b + len) = '\0';

    return *this;
  }

  // Password: text, 32 bytes; offset 64
  [[nodiscard]] static constexpr size_t PasswordEncodingLength() noexcept {
    return 32;
  }

  [[nodiscard]] static constexpr size_t PasswordEncodingOffset() noexcept {
    return 64;
  }

  [[nodiscard]] std::string Password() const noexcept {
    const char *b = buffer_ + buffer_offset_ + PasswordEncodingOffset();
    return std::string{b};
  }

  LoginRequest &Password(const std::string &val) noexcept {
    char *b = buffer_ + buffer_offset_ + PasswordEncodingOffset();
    size_t max_length = PasswordEncodingLength() - 1;
    size_t len = val.size();
    if (len > max_length) len = max_length;

    memcpy(b, val.c_str(), len);
    *(b + len) = '\0';

    return *this;
  }

 private:
  char *buffer_;
  size_t buffer_length_;
  size_t buffer_offset_;
};
