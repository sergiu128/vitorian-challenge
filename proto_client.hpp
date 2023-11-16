#pragma once

#include "msg_header.hpp"
#include "resolver.hpp"
#include "tcp_client.hpp"
#include "tcp_stream.hpp"

class ProtoClient {
 public:
  ProtoClient() = default;
  ~ProtoClient() = default;

  // Copy.
  ProtoClient(const ProtoClient&) = delete;
  ProtoClient& operator=(const ProtoClient&) = delete;

  // Move.
  ProtoClient(ProtoClient&&) = delete;
  ProtoClient& operator=(ProtoClient&&) = delete;

  bool Run(const char* addr, int port);
  bool Run(const std::vector<Resolver::Addr>& addrs);
  bool RunOne(const Resolver::Addr& addr);

  std::string Token() const noexcept { return token_; }

 private:
  TcpClient client_{};
  Resolver resolver_{};
  std::string token_{};
};
