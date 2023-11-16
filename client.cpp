#include <iostream>

#include "proto_client.hpp"

int main() {
  ProtoClient client{};
  try {
    client.Run("challenge1.vitorian.com", 9009);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
