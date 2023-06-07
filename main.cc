#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "rng.h"

int main() {
  std::vector<uint32_t> outputs = {
      1, 1, 2, 3, 4, 5, 6, 7, 1,
  };

  std::vector<uint32_t> mods = {
      7, 2, 6, 5, 4, 3, 2, 1, 2,
  };

  const std::unordered_map<uint64_t, Derivatives>& results =
      find_derivatives(outputs, mods);

  (void)results;

  free_derivatives();
}