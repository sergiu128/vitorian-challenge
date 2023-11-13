#include "resolver.hpp"

#include <arpa/inet.h>
#include <netdb.h>

#include <cstring>
#include <iostream>

Resolver::~Resolver() { Close(); }

std::vector<Resolver::Addr> Resolver::Resolve(const char* addr, int port) {
  Close();

  std::cout << "(resolver) resolving addr=" << addr << " port=" << port
            << std::endl;

  std::vector<Resolver::Addr> results{};

  struct addrinfo hint {};
  memset(&hint, 0, sizeof(addrinfo));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;

  std::string port_str{std::to_string(port)};

  if (getaddrinfo(addr, port_str.c_str(), &hint, &ips_) != 0) {
    std::cout << "ERR(resolver) getaddrinfo" << std::endl;
    throw std::system_error(errno, std::generic_category());
  }

  const char* ip_str{nullptr};
  for (struct addrinfo* ip = ips_; ip != nullptr; ip = ip->ai_next) {
    void* ip_addr{};

    if (ip->ai_family == AF_INET) {
      struct sockaddr_in* ipv4 = (struct sockaddr_in*)ip->ai_addr;
      ip_addr = &(ipv4->sin_addr);
    } else if (ip->ai_family == AF_INET6) {
      struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)ip->ai_addr;
      ip_addr = &(ipv6->sin6_addr);
    }

    ip_str =
        inet_ntop(ip->ai_family, ip_addr, ip_str_storage_, INET6_ADDRSTRLEN);
    if (ip_str == nullptr) {
      continue;
    }

    std::cout << "(resolver) addr=" << addr << " resolved to " << ip_str
              << std::endl;

    results.push_back(Resolver::Addr{
        .socket_domain = ip->ai_family,
        .socket_type = ip->ai_socktype,
        .socket_protocol = ip->ai_protocol,
        .addr = ip->ai_addr,
        .addr_len = ip->ai_addrlen,
        .addr_str = std::string{ip_str},
        .port = port,
    });
  }

  return results;
}

void Resolver::Close() noexcept {
  if (ips_) {
    freeaddrinfo(ips_);
    ips_ = nullptr;
  }
}
