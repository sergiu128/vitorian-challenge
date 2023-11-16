#pragma once
#include "resolver.hpp"

// ConnectWithTimeout returns true if we can connect with the given socket and
// resolved address within timeout_s. Otherwise, returns false.
bool ConnectWithTimeout(int sockfd, const Resolver::Addr& addr,
                        int timeout_s = 1);
