#include "rng.h"

#include <assert.h>

#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <stack>
#include <thread>
#include <unordered_map>
#include <vector>

// E.g. -DTHREAD_COUNT=8
static_assert(THREAD_COUNT > 0);

namespace {

constexpr uint32_t kThirtyTwoOnes = 0xFFFFFFFF;

std::unordered_map<uint64_t, Derivatives> cache;
std::mutex cache_mutex;

std::queue<uint64_t> next_to_process;
std::mutex next_to_process_mutex;

void consume_and_calculate_derivatives() {
  uint64_t process_me = 0;
  {
    std::lock_guard<std::mutex> guard(next_to_process_mutex);
    process_me = next_to_process.front();
    next_to_process.pop();
  }

  const uint32_t output = get_output(process_me);
  const uint32_t mod = get_mod(process_me);

  const uint32_t num_derivatives = kThirtyTwoOnes / mod;

  uint32_t* derivative_values =
      static_cast<uint32_t*>(malloc(sizeof(uint32_t) * num_derivatives));
  assert(derivative_values != nullptr);

  derivative_values[0] = output;

  for (uint64_t i = 1; i < num_derivatives; ++i) {
    derivative_values[i] = derivative_values[i - 1] + mod;
  }

  {
    std::lock_guard<std::mutex> guard(cache_mutex);
    cache[process_me] = {
        .length = num_derivatives,
        .values = derivative_values,
    };
  }
}
}  // namespace

std::unordered_map<uint64_t, Derivatives>& find_derivatives(
    const std::vector<uint32_t>& outputs, const std::vector<uint32_t>& mods) {
  assert(outputs.size() == mods.size());

  const int num_inputs = static_cast<int>(outputs.size());
  for (int i = 0; i < num_inputs; ++i) {
    assert(mods[i] != 0);
    uint64_t k = create_key(outputs[i], mods[i]);
    if (cache.find(k) != cache.end()) {
      continue;
    }

    cache[k] = {
        .length = 0,
        .values = nullptr,
    };

    next_to_process.push(k);
  }

  const int num_threads = num_inputs < THREAD_COUNT ? num_inputs : THREAD_COUNT;

  std::stack<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.push(std::thread(consume_and_calculate_derivatives));
  }

  while (threads.empty() == false) {
    threads.top().join();
    threads.pop();
  }

  return cache;
}

void free_derivatives() {
  for (const auto& [k, derivs] : cache) {
    if (derivs.values != nullptr) {
      free(derivs.values);
    }
  }
  cache.clear();
}