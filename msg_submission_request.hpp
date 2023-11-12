#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

class SubmissionRequest {
 public:
  SubmissionRequest() = delete;
  SubmissionRequest(char *buffer, size_t buffer_length, size_t buffer_offset)
      : buffer_{buffer},
        buffer_length_{buffer_length},
        buffer_offset_{buffer_offset} {
    if (buffer_length < buffer_offset + EncodedLength()) {
      throw std::runtime_error("buffer to small for SubmissionRequest");
    }
  }
  ~SubmissionRequest() = default;

  // Copy.
  SubmissionRequest(const SubmissionRequest &) = default;
  SubmissionRequest &operator=(const SubmissionRequest &) = default;

  // Move.
  SubmissionRequest(SubmissionRequest &&) = default;
  SubmissionRequest &operator=(SubmissionRequest &&) = default;

  [[nodiscard]] static constexpr char MsgType() noexcept { return 'S'; }

  [[nodiscard]] static constexpr size_t EncodedLength() noexcept {
    return 3 * 64;
  }

  [[nodiscard]] const char *Buffer() const noexcept { return buffer_; }
  [[nodiscard]] size_t BufferLength() const noexcept { return buffer_length_; }
  [[nodiscard]] size_t BufferOffset() const noexcept { return buffer_offset_; }

  // Name: text, 64 bytes, offset 0
  [[nodiscard]] static constexpr size_t NameEncodingLength() noexcept {
    return 64;
  }

  [[nodiscard]] static constexpr size_t NameEncodingOffset() noexcept {
    return 0;
  }

  [[nodiscard]] const char *Name() const noexcept {
    return buffer_ + buffer_offset_ + NameEncodingOffset();
  }

  template <typename ClaimFnT>
  SubmissionRequest &Name(ClaimFnT &&fn) noexcept {
    char *b = buffer_ + buffer_offset_ + NameEncodingOffset();
    size_t max_length = NameEncodingLength() - 1;
    size_t n_used = fn(b, max_length);

    if (n_used > max_length) {
      n_used = max_length;
    }
    *(b + n_used) = '\0';

    return *this;
  }

  // Email: text, 64 bytes; offset 64
  [[nodiscard]] static constexpr size_t EmailEncodingLength() noexcept {
    return 64;
  }

  [[nodiscard]] static constexpr size_t EmailEncodingOffset() noexcept {
    return 64;
  }

  [[nodiscard]] const char *Email() const noexcept {
    return buffer_ + buffer_offset_ + EmailEncodingOffset();
  }

  template <typename ClaimFnT>
  SubmissionRequest &Email(ClaimFnT &&fn) noexcept {
    char *b = buffer_ + buffer_offset_ + EmailEncodingOffset();
    size_t max_length = EmailEncodingLength() - 1;

    size_t n_used = fn(b, max_length);
    if (n_used > max_length) {
      n_used = max_length;
    }
    *(b + n_used) = '\0';

    return *this;
  }

  // Repo: text, 64 bytes; offset 128
  [[nodiscard]] static constexpr size_t RepoEncodingLength() noexcept {
    return 64;
  }

  [[nodiscard]] static constexpr size_t RepoEncodingOffset() noexcept {
    return 128;
  }

  [[nodiscard]] const char *Repo() const noexcept {
    return buffer_ + buffer_offset_ + RepoEncodingOffset();
  }

  template <typename ClaimFnT>
  SubmissionRequest &Repo(ClaimFnT &&fn) noexcept {
    char *b = buffer_ + buffer_offset_ + RepoEncodingOffset();
    size_t max_length = RepoEncodingLength() - 1;

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
