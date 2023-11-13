#pragma once

#include <cstddef>

class TcpStream {
 public:
  TcpStream() = delete;
  TcpStream(int sockfd);
  ~TcpStream();

  // Copy.
  TcpStream(const TcpStream&) = delete;
  TcpStream& operator=(const TcpStream&) = delete;

  // Move.
  TcpStream(TcpStream&&);
  TcpStream& operator=(TcpStream&&);

  void Close() noexcept;
  void ReadExact(char* b, size_t len);
  void WriteExact(const char* b, size_t len);

 private:
  int sockfd_;
};
