#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
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

  [[nodiscard]] std::string Name() const noexcept {
    const char *b = buffer_ + buffer_offset_ + NameEncodingOffset();
    return std::string{b};
  }

  SubmissionRequest &Name(const std::string &val) noexcept {
    char *b = buffer_ + buffer_offset_ + NameEncodingOffset();
    size_t len = std::min(val.size(), NameEncodingLength());

    memcpy(b, val.c_str(), len);
    memset(b + len, 0, NameEncodingLength() - len);

    return *this;
  }

  // Email: text, 64 bytes; offset 64
  [[nodiscard]] static constexpr size_t EmailEncodingLength() noexcept {
    return 64;
  }

  [[nodiscard]] static constexpr size_t EmailEncodingOffset() noexcept {
    return 64;
  }

  [[nodiscard]] std::string Email() const noexcept {
    const char *b = buffer_ + buffer_offset_ + EmailEncodingOffset();
    return std::string{b};
  }

  SubmissionRequest &Email(const std::string &val) noexcept {
    char *b = buffer_ + buffer_offset_ + EmailEncodingOffset();
    size_t len = std::min(val.size(), EmailEncodingLength());

    memcpy(b, val.c_str(), len);
    memset(b + len, 0, EmailEncodingLength() - len);

    return *this;
  }

  // Repo: text, 64 bytes; offset 128
  [[nodiscard]] static constexpr size_t RepoEncodingLength() noexcept {
    return 64;
  }

  [[nodiscard]] static constexpr size_t RepoEncodingOffset() noexcept {
    return 128;
  }

  [[nodiscard]] std::string Repo() const noexcept {
    const char *b = buffer_ + buffer_offset_ + RepoEncodingOffset();
    return std::string{b};
  }

  SubmissionRequest &Repo(const std::string &val) noexcept {
    char *b = buffer_ + buffer_offset_ + RepoEncodingOffset();
    size_t len = std::min(val.size(), RepoEncodingLength());

    memcpy(b, val.c_str(), len);
    memset(b + len, 0, RepoEncodingLength() - len);

    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const SubmissionRequest &req) {
    os << "submission_request name=" << req.Name() << " email=" << req.Email()
       << " repo=" << req.Repo();
    return os;
  }

 private:
  char *buffer_;
  size_t buffer_length_;
  size_t buffer_offset_;
};
