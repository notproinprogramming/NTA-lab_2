#include "algorithms.hpp"

#include <chrono>
#include <iostream>

// --- Перебір ---

int64_t brute_force_dlog(int64_t alpha, int64_t beta, int64_t p, int timeout_sec) {
  auto start = std::chrono::steady_clock::now();
  int64_t cur = 1;
  int64_t n = p - 1;

  for (int64_t x = 0; x < n; ++x) {
    if (cur == beta) return x;
    cur = (__int128)cur * alpha % p;

    if (x % 10000 == 0) {
      auto now = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
      if (elapsed >= timeout_sec) {
        std::cerr << "Brute force: timeout after " << timeout_sec << "s\n";
        return -1;
      }
    }
  }
  return -1;
}

// --- Допоміжні функції ---

int64_t mod_pow(int64_t base, int64_t exp, int64_t mod) {
  int64_t result = 1;
  base %= mod;
  while (exp > 0) {
    if (exp & 1) result = (__int128)result * base % mod;
    base = (__int128)base * base % mod;
    exp >>= 1;
  }
  return result;
}
