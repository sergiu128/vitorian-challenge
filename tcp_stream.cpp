#include "tcp_stream.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <system_error>

TcpStream::TcpStream(int sockfd) : sockfd_{sockfd} {
  struct timeval timeout {
    .tv_sec = 5, .tv_usec = 0,
  };
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) !=
      0) {
    std::cout << "ERR(tcp-stream) could not set recv timeout" << std::endl;
    Close();
    throw std::system_error(errno, std::generic_category());
  }
  if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) !=
      0) {
    std::cout << "ERR(tcp-stream) could not set send timeout" << std::endl;
    Close();
    throw std::system_error(errno, std::generic_category());
  }

  std::cout << "(tcp-stream) created fd=" << sockfd << std::endl;
}

TcpStream::~TcpStream() { Close(); }

void TcpStream::Close() noexcept {
  if (sockfd_ >= 0) {
    std::cout << "(tcp-stream) closed fd=" << sockfd_ << std::endl;
    close(sockfd_);
    sockfd_ = -1;
  }
}

void TcpStream::ReadExact(char* b, size_t len) {
  if (sockfd_ <= 0) {
    throw std::runtime_error("cannot read from an unitialized stream");
  }

  size_t n_sofar{0};
  while (n_sofar < len) {
    int n_now = read(sockfd_, b + n_sofar, len - n_sofar);
    if (n_now < 0) {
      throw std::system_error(errno, std::generic_category());
    } else if (n_now == 0) {
      throw std::runtime_error("peer has shutdown");
    }
    n_sofar += static_cast<size_t>(n_now);
  }
}

void TcpStream::WriteExact(const char* b, size_t len) {
  if (sockfd_ <= 0) {
    throw std::runtime_error("cannot read from an unitialized stream");
  }

  size_t n_sofar{0};
  while (n_sofar < len) {
    int n_now = write(sockfd_, b + n_sofar, len - n_sofar);
    if (n_now < 0) {
      throw std::system_error(errno, std::generic_category());
    }
    n_sofar += static_cast<size_t>(n_now);
  }
}
