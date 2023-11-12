#include "tcp_client.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>

TcpStream TcpClient::Connect(const char* addr, int port) {
  std::cout << "(tcp-client) trying to connect to addr=" << addr
            << " port=" << port << std::endl;

  struct addrinfo hint {};
  memset(&hint, 0, sizeof(addrinfo));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;

  std::string port_str{std::to_string(port)};

  struct addrinfo* ips{};
  if (getaddrinfo(addr, port_str.c_str(), &hint, &ips) != 0) {
    std::cout << "ERR(tcp-client) getaddrinfo" << std::endl;
    throw std::system_error(errno, std::generic_category());
  }

  char ip_str_storage[INET6_ADDRSTRLEN];
  const char* ip_str{nullptr};
  int sockfd{-1};
  for (struct addrinfo* ip = ips; ip != nullptr; ip = ip->ai_next) {
    void* ip_addr{};

    if (ip->ai_family == AF_INET) {
      struct sockaddr_in* ipv4 = (struct sockaddr_in*)ip->ai_addr;
      ip_addr = &(ipv4->sin_addr);
    } else if (ip->ai_family == AF_INET6) {
      struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)ip->ai_addr;
      ip_addr = &(ipv6->sin6_addr);
    }

    ip_str =
        inet_ntop(ip->ai_family, ip_addr, ip_str_storage, INET6_ADDRSTRLEN);
    if (ip_str == nullptr) {
      continue;
    }

    std::cout << "(tcp-client) addr=" << addr << " resolved to " << ip_str
              << ", trying to connect" << std::endl;

    sockfd = socket(ip->ai_family, ip->ai_socktype, ip->ai_protocol);
    if (sockfd == -1) {
      std::cout << "(tcp-client) could not create socket for " << ip_str_storage
                << std::endl;
      continue;
    }

    if (connect(sockfd, ip->ai_addr, ip->ai_addrlen) != 0) {
      std::cout << "(tcp-client) could not connect to " << ip_str_storage
                << std::endl;
      close(sockfd);
      sockfd = -1;
      continue;
    }
  }

  if (sockfd == -1) {
    freeaddrinfo(ips);
    std::cout << "ERR(tcp-client) socket" << std::endl;
    throw std::runtime_error("(tcp-client) could not connect to any ips");
  }

  std::cout << "(tcp-client) connected to addr=" << addr << " ip=" << ip_str
            << " fd=" << sockfd << std::endl;

  freeaddrinfo(ips);

  return TcpStream{sockfd};
}
