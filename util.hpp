#pragma once

#include <ctime>
#include <cstdint>

inline int64_t EpochNanos() {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return ts.tv_sec * 1'000'000'000LL + ts.tv_nsec;
}
