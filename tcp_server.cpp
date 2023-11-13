#include "tcp_server.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <system_error>

TcpServer::TcpServer(const Resolver::Addr& addr) {
  int sockfd =
      socket(addr.socket_domain, addr.socket_type, addr.socket_protocol);
  if (sockfd < 0) {
    std::cout << "ERR(tcp-server) socket" << std::endl;
    throw std::system_error(errno, std::generic_category());
  }

  int enable{1};
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) != 0) {
    std::cout << "ERR(tcp-server) setsockopt SO_REUSEADDR" << std::endl;
    throw std::system_error(errno, std::generic_category());
  }

  if (bind(sockfd, addr.addr, addr.addr_len) != 0) {
    std::cout << "ERR(tcp-server) bind" << std::endl;
    close(sockfd);
    throw std::system_error(errno, std::generic_category());
  }

  if (listen(sockfd, 128) != 0) {
    std::cout << "ERR(tcp-server) listen" << std::endl;
    close(sockfd);
    throw std::system_error(errno, std::generic_category());
  }

  sockfd_ = sockfd;

  std::cout << "(tcp-server) listening on " << addr.addr_str
            << " fd=" << sockfd_ << std::endl;
}

TcpServer::~TcpServer() { Close(); }

TcpServer::TcpServer(TcpServer&& from) {
  this->sockfd_ = from.sockfd_;
  from.sockfd_ = -1;
}

TcpServer& TcpServer::operator=(TcpServer&& from) {
  Close();
  this->sockfd_ = from.sockfd_;
  from.sockfd_ = -1;
  return *this;
}

void TcpServer::Close() noexcept {
  if (sockfd_ >= 0) {
    std::cout << "(tcp-server) closing listen socket fd=" << sockfd_
              << std::endl;
    close(sockfd_);
    sockfd_ = -1;
  }
}

TcpStream TcpServer::Accept() {
  struct sockaddr_storage peer_addr {};
  socklen_t addrlen;
  int sockfd = accept(sockfd_, (struct sockaddr*)(&peer_addr), &addrlen);
  if (sockfd < 0) {
    std::cout << "ERR(tcp-server) accept" << std::endl;
    throw std::system_error(errno, std::generic_category());
  }

  std::cout << "(tcp-server) accepted connection fd=" << sockfd << std::endl;

  return TcpStream{sockfd};
}
