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

  [[nodiscard]] const char *User() const noexcept {
    return buffer_ + buffer_offset_ + UserEncodingOffset();
  }

  template <typename ClaimFnT>
  LoginRequest &User(ClaimFnT &&fn) noexcept {
    char *b = buffer_ + buffer_offset_ + UserEncodingOffset();
    size_t max_length = UserEncodingLength() - 1;
    size_t n_used = fn(b, max_length);

    if (n_used > max_length) {
      n_used = max_length;
    }
    *(b + n_used) = '\0';

    return *this;
  }

  // Password: text, 32 bytes; offset 64
  [[nodiscard]] static constexpr size_t PasswordEncodingLength() noexcept {
    return 32;
  }

  [[nodiscard]] static constexpr size_t PasswordEncodingOffset() noexcept {
    return 64;
  }

  [[nodiscard]] const char *Password() const noexcept {
    return buffer_ + buffer_offset_ + PasswordEncodingOffset();
  }

  template <typename ClaimFnT>
  LoginRequest &Password(ClaimFnT &&fn) noexcept {
    char *b = buffer_ + buffer_offset_ + PasswordEncodingOffset();
    size_t max_length = PasswordEncodingLength() - 1;

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
