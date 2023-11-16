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

#include "net.hpp"

TcpStream TcpClient::Connect(const Resolver::Addr& addr) {
  std::cout << "(tcp-client) trying to connect to addr=" << addr.addr_str
            << " port=" << addr.port << std::endl;

  int sockfd{-1};
  sockfd = socket(addr.socket_domain, addr.socket_type, addr.socket_protocol);
  if (sockfd == -1) {
    std::cout << "ERR(tcp-client) could not create socket for " << addr.addr_str
              << std::endl;
    throw std::system_error(errno, std::generic_category());
  }

  if (!ConnectWithTimeout(sockfd, addr)) {
    std::cout << "ERR(tcp-client) could not connect to " << addr.addr_str
              << std::endl;
    close(sockfd);
    sockfd = -1;
    throw std::system_error(errno, std::generic_category());
  }

  std::cout << "(tcp-client) connected to addr=" << addr.addr_str
            << " fd=" << sockfd << std::endl;

  return TcpStream{sockfd};
}
