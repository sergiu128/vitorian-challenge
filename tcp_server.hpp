#pragma once

#include "resolver.hpp"
#include "tcp_stream.hpp"

class TcpServer {
 public:
  TcpServer(const char* addr, int port);
  ~TcpServer();

  // Copy.
  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;

  // Move.
  TcpServer(TcpServer&&);
  TcpServer& operator=(TcpServer&&);

  void Close() noexcept;
  TcpStream Accept();

 private:
  int sockfd_{-1};
  Resolver resolver_{};
};
