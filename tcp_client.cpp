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

  auto addrs = resolver_.Resolve(addr, port);
  if (addrs.size() == 0) {
    throw std::runtime_error("could not resolve addr to any ips");
  }

  int sockfd{-1};
  for (const auto& resolved_addr : addrs) {
    sockfd = socket(resolved_addr.socket_domain, resolved_addr.socket_type,
                    resolved_addr.socket_protocol);
    if (sockfd == -1) {
      std::cout << "(tcp-client) could not create socket for "
                << resolved_addr.addr_str << std::endl;
      continue;
    }

    if (connect(sockfd, resolved_addr.addr, resolved_addr.addr_len) != 0) {
      std::cout << "(tcp-client) could not connect to "
                << resolved_addr.addr_str << std::endl;
      close(sockfd);
      sockfd = -1;
      continue;
    } else {
      std::cout << "(tcp-client) connected to addr=" << addr
                << " ip=" << resolved_addr.addr_str << " fd=" << sockfd
                << std::endl;
      return TcpStream{sockfd};
    }
  }

  if (sockfd == -1) {
    std::cout << "ERR(tcp-client) socket" << std::endl;
    throw std::runtime_error("(tcp-client) could not connect to any ips");
  }

  return TcpStream{sockfd};
}
