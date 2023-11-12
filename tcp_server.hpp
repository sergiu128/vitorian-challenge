#pragma once

#include "tcp_stream.hpp"

class TcpServer {
 public:
  TcpServer(const char* addr, int port);
  ~TcpServer();

  // Copy.
  TcpServer(const TcpServer&) = default;
  TcpServer& operator=(const TcpServer&) = default;

  // Move.
  TcpServer(TcpServer&&) = default;
  TcpServer& operator=(TcpServer&&) = default;

  void Close() noexcept;
  TcpStream Accept();

 private:
  int sockfd_{-1};
};
