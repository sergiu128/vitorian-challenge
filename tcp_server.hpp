#pragma once

#include "resolver.hpp"
#include "tcp_stream.hpp"

class TcpServer {
 public:
  TcpServer() = delete;
  TcpServer(const Resolver::Addr& addr);
  ~TcpServer();

  // Copy.
  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;

  // Move.
  TcpServer(TcpServer&&);
  TcpServer& operator=(TcpServer&&);

  void Close() noexcept;
  [[nodiscard]] TcpStream Accept();

 private:
  int sockfd_{-1};
};
