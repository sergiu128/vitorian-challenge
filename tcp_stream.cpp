#include "tcp_stream.hpp"

#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <system_error>

TcpStream::TcpStream(int fd) : fd_{fd} {
  std::cout << "(tcp-stream) created fd=" << fd << std::endl;
}

TcpStream::~TcpStream() { Close(); }

void TcpStream::Close() noexcept {
  if (fd_ >= 0) {
    std::cout << "(tcp-stream) closed" << std::endl;
    close(fd_);
    fd_ = -1;
  }
}

void TcpStream::ReadExact(char* b, size_t len) {
  if (fd_ <= 0) {
    throw std::runtime_error("cannot read from an unitialized stream");
  }

  size_t n_sofar{0};
  while (n_sofar < len) {
    int n_now = read(fd_, b + n_sofar, len - n_sofar);
    if (n_now < 0) {
      throw std::system_error(errno, std::generic_category());
    } else if (n_now == 0) {
      throw std::runtime_error("peer has shutdown");
    }
    n_sofar += static_cast<size_t>(n_now);
  }
}

void TcpStream::WriteExact(const char* b, size_t len) {
  if (fd_ <= 0) {
    throw std::runtime_error("cannot read from an unitialized stream");
  }

  size_t n_sofar{0};
  while (n_sofar < len) {
    int n_now = write(fd_, b + n_sofar, len - n_sofar);
    if (n_now < 0) {
      throw std::system_error(errno, std::generic_category());
    }
    n_sofar += static_cast<size_t>(n_now);
  }
}
