#pragma once

#include <netdb.h>

#include <string>
#include <vector>

class Resolver {
 public:
  Resolver() = default;
  ~Resolver();

  // Copy.
  Resolver(const Resolver&) = default;
  Resolver& operator=(const Resolver&) = default;

  // Move.
  Resolver(Resolver&&) = default;
  Resolver& operator=(Resolver&&) = default;

  struct Addr {
    int socket_domain;
    int socket_type;
    int socket_protocol;
    struct sockaddr* addr;
    socklen_t addr_len;
    std::string addr_str;
    int port;
  };
  [[nodiscard]] std::vector<Addr> Resolve(const char* addr, int port);

  void Close() noexcept;

 private:
  char ip_str_storage_[INET6_ADDRSTRLEN];
  struct addrinfo* ips_{nullptr};
};
