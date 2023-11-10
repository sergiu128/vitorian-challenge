#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

class SubmissionResponse {
 public:
  SubmissionResponse() = delete;
  SubmissionResponse(char *buffer, size_t buffer_length, size_t buffer_offset)
      : buffer_{buffer},
        buffer_length_{buffer_length},
        buffer_offset_{buffer_offset} {
    if (buffer_length < buffer_offset + EncodedLength()) {
      throw std::runtime_error("buffer to small for SubmissionResponse");
    }
  }
  ~SubmissionResponse() = default;

  // Copy.
  SubmissionResponse(const SubmissionResponse &) = default;
  SubmissionResponse &operator=(const SubmissionResponse &) = default;

  // Move.
  SubmissionResponse(SubmissionResponse &&) = default;
  SubmissionResponse &operator=(SubmissionResponse &&) = default;

  [[nodiscard]] char MsgType() const noexcept { return 'R'; }

  [[nodiscard]] static constexpr size_t EncodedLength() noexcept { return 32; }

  [[nodiscard]] const char *Buffer() const noexcept { return buffer_; }
  [[nodiscard]] size_t BufferLength() const noexcept { return buffer_length_; }
  [[nodiscard]] size_t BufferOffset() const noexcept { return buffer_offset_; }

  // Token: text, 32 bytes, offset 0
  [[nodiscard]] static constexpr size_t TokenEncodingLength() noexcept {
    return 32;
  }

  [[nodiscard]] static constexpr size_t TokenEncodingOffset() noexcept {
    return 0;
  }

  [[nodiscard]] const char *Token() const noexcept {
    return buffer_ + buffer_offset_ + TokenEncodingOffset();
  }

  template <typename ClaimFnT>
  SubmissionResponse &Token(ClaimFnT &&fn) noexcept {
    char *b = buffer_ + buffer_offset_ + TokenEncodingOffset();
    size_t max_length = TokenEncodingLength() - 1;
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
