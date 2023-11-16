#include "net.hpp"

#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <system_error>

bool ConnectWithTimeout(int sockfd, const Resolver::Addr& addr, int timeout_s) {
  int flags = fcntl(sockfd, F_GETFL, 0);
  if (flags < 0) {
    throw std::system_error(errno, std::generic_category());
  }

  if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
    throw std::system_error(errno, std::generic_category());
  }

  if (connect(sockfd, addr.addr, addr.addr_len) != 0) {
    if (errno != EINPROGRESS) {
      throw std::system_error(errno, std::generic_category());
    }
  }

  fd_set writefds;
  FD_ZERO(&writefds);
  FD_SET(sockfd, &writefds);

  struct timeval timeout {
    .tv_sec = timeout_s, .tv_usec = 0
  };

  int ret = select(sockfd + 1, NULL, &writefds, NULL, &timeout);
  if (ret == -1) throw std::system_error(errno, std::generic_category());
  if (ret == 0) return false;

  if (fcntl(sockfd, F_SETFL, flags) == -1) {
    throw std::system_error(errno, std::generic_category());
  }

  return true;
}
