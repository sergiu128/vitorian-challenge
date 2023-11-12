#pragma once

#include "tcp_stream.hpp"

class TcpClient {
 public:
  TcpClient() = default;
  ~TcpClient() = default;

  // Copy.
  TcpClient(const TcpClient&) = default;
  TcpClient& operator=(const TcpClient&) = default;

  // Move.
  TcpClient(TcpClient&&) = default;
  TcpClient& operator=(TcpClient&&) = default;

  TcpStream Connect(const char* addr, int port);

 private:
};
