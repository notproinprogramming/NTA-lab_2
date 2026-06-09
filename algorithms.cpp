#include "algorithms.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
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

int64_t mod_inv(int64_t a, int64_t mod) {
  int64_t g = mod, x = 0, y = 1;
  int64_t ta = a;
  while (ta != 0) {
    int64_t q = g / ta;
    g -= q * ta;
    std::swap(g, ta);
    x -= q * y;
    std::swap(x, y);
  }
  if (g != 1) throw std::runtime_error("mod_inv: gcd != 1, inverse does not exist");
  return (x % mod + mod) % mod;
}

std::vector<std::pair<int64_t, int64_t>> factorize(int64_t n) {
  std::vector<std::pair<int64_t, int64_t>> factors;
  for (int64_t d = 2; d * d <= n; ++d) {
    if (n % d == 0) {
      int64_t exp = 0;
      while (n % d == 0) {
        ++exp;
        n /= d;
      }
      factors.push_back({d, exp});
    }
  }
  if (n > 1) factors.push_back({n, 1});
  return factors;
}

int64_t crt(const std::vector<int64_t>& r, const std::vector<int64_t>& m) {
  // модулі мають бути попарно взаємно прості
  int64_t M = 1;
  for (int64_t mi : m) M *= mi;

  int64_t x = 0;
  for (size_t i = 0; i < r.size(); ++i) {
    int64_t Mi = M / m[i];
    int64_t yi = mod_inv(Mi % m[i], m[i]);
    x = (x + (__int128)r[i] % M * Mi % M * yi) % M;
  }
  return (x + M) % M;
}

// --- СПГ ---

// Знаходить x mod pi^li, де alpha^x = beta (mod p)
static int64_t dlog_prime_power(int64_t alpha, int64_t beta, int64_t p, int64_t pi, int64_t li) {
  int64_t n = p - 1;

  // таблиця: r_{pi,j} = alpha^(n/pi * j), j = 0..pi-1
  std::unordered_map<int64_t, int64_t> table;
  int64_t base = mod_pow(alpha, n / pi, p);
  int64_t cur = 1;
  for (int64_t j = 0; j < pi; ++j) {
    table[cur] = j;
    cur = (__int128)cur * base % p;
  }

  int64_t pi_l = 1;
  for (int64_t k = 0; k < li; ++k) pi_l *= pi;

  int64_t x = 0;
  int64_t gamma = beta;  // beta * alpha^(-x_accumulated)
  int64_t pi_k = 1;      // pi^k

  for (int64_t k = 0; k < li; ++k) {
    // lhs = gamma^(n / pi^(k+1)) - має збігатися з якимось r_{pi,j}
    int64_t lhs = mod_pow(gamma, n / (pi_k * pi), p);

    auto it = table.find(lhs);
    if (it == table.end()) return -1;

    int64_t xk = it->second;
    x += xk * pi_k;

    // оновлюємо gamma = beta * alpha^(-x)
    int64_t inv_alpha_x = mod_pow(mod_inv(alpha, p), x, p);
    gamma = (__int128)beta * inv_alpha_x % p;

    pi_k *= pi;
  }

  return x % pi_l;
}

int64_t pohlig_hellman(int64_t alpha, int64_t beta, int64_t p) {
  int64_t n = p - 1;
  auto factors = factorize(n);

  std::vector<int64_t> remainders;
  std::vector<int64_t> moduli;

  for (auto [pi, li] : factors) {
    int64_t pi_l = 1;
    for (int64_t k = 0; k < li; ++k) pi_l *= pi;

    int64_t xi = dlog_prime_power(alpha, beta, p, pi, li);
    if (xi < 0) return -1;

    remainders.push_back(xi);
    moduli.push_back(pi_l);
  }

  return crt(remainders, moduli);
}
