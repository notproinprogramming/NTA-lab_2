#include <chrono>
#include <cstdlib>
#include <iostream>

#include "algorithms.hpp"

static void print_usage(const char* argv0) {
  std::cerr << "Usage:\n"
            << "  " << argv0 << " 1 <alpha> <beta> <p>   - brute force\n"
            << "  " << argv0 << " 2 <alpha> <beta> <p>   - Pohlig-Hellman\n"
            << "  " << argv0 << " 3                      - demo on test cases\n";
}

static void run_brute_force(int64_t alpha, int64_t beta, int64_t p) {
  std::cout << "[Brute force] alpha=" << alpha << ", beta=" << beta << ", p=" << p << "\n";

  auto t0 = std::chrono::steady_clock::now();
  int64_t x = brute_force_dlog(alpha, beta, p);
  auto t1 = std::chrono::steady_clock::now();
  double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

  if (x >= 0) {
    std::cout << "x = " << x << "  (" << ms << " ms)\n";
    int64_t check = mod_pow(alpha, x, p);
    std::cout << "Verification: " << alpha << "^" << x << " mod " << p << " = " << check << (check == beta ? "  OK" : "  FAIL") << "\n";
  } else {
    std::cout << "Not found (timeout or no solution)\n";
  }
}

static void run_demo() {
  // тестові значення: p - просте, alpha - примітивний корінь, 100 <= x <= 999
  struct Case {
    int64_t p, alpha, beta, x;
  };
  static const Case cases[] = {
      {503, 5, 498, 754},
      {509, 2, 379, 214},
      {521, 3, 61, 125},
  };

  for (const auto& c : cases) {
    std::cout << "-------------------------------\n";
    std::cout << "Expected x = " << c.x << "\n";
    run_brute_force(c.alpha, c.beta, c.p);
    // run_pohlig_hellman(c.alpha, c.beta, c.p);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  int method = std::atoi(argv[1]);

  if (method == 3) {
    run_demo();
    return 0;
  }

  if (argc != 5) {
    print_usage(argv[0]);
    return 1;
  }

  int64_t alpha = std::atoll(argv[2]);
  int64_t beta = std::atoll(argv[3]);
  int64_t p = std::atoll(argv[4]);

  switch (method) {
    case 1:
      run_brute_force(alpha, beta, p);
      break;
    // case 2: run_pohlig_hellman(alpha, beta, p); break;
    default:
      std::cerr << "Unknown method: " << method << "\n";
      print_usage(argv[0]);
      return 1;
  }

  return 0;
}
