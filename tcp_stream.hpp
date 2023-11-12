#pragma once

#include <cstddef>

class TcpStream {
 public:
  TcpStream() = delete;
  TcpStream(int fd);
  ~TcpStream();

  // Copy.
  TcpStream(const TcpStream&) = default;
  TcpStream& operator=(const TcpStream&) = default;

  // Move.
  TcpStream(TcpStream&&) = default;
  TcpStream& operator=(TcpStream&&) = default;

  void Close() noexcept;
  void ReadExact(char* b, size_t len);
  void WriteExact(const char* b, size_t len);

 private:
  int fd_;
};
