#ifndef RNG_H
#define RNG_H

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

inline uint64_t create_key(uint32_t output, uint32_t mod) {
  return (static_cast<uint64_t>(output) << 32) | mod;
}

inline uint32_t get_output(uint64_t k) {
  return static_cast<uint32_t>(k >> 32);
}

inline uint32_t get_mod(uint64_t k) {
  return static_cast<uint32_t>((k << 32) >> 32);
}

struct Derivatives {
  std::size_t length;
  uint32_t* values;
};

std::unordered_map<uint64_t, Derivatives>& find_derivatives(
    const std::vector<uint32_t>& outputs, const std::vector<uint32_t>& mods);

void free_derivatives();

#endif  // RNG_H