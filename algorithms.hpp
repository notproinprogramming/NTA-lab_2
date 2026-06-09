#pragma once

#include <cstdint>
#include <utility>
#include <vector>

// Дискретний логарифм перебором
// Повертає x або -1 при таймауті / відсутності розв'язку
int64_t brute_force_dlog(int64_t alpha, int64_t beta, int64_t p, int timeout_sec = 300);

// Піднесення до степеня за модулем
int64_t mod_pow(int64_t base, int64_t exp, int64_t mod);

// Обернений елемент за модулем (розширений алгоритм Евкліда)
int64_t mod_inv(int64_t a, int64_t mod);

// Канонічний розклад n на прості множники: повертає пари (prime, exponent)
std::vector<std::pair<int64_t, int64_t>> factorize(int64_t n);

// Китайська теорема про лишки
// Розв'язує систему x = r[i] (mod m[i]), повертає x mod M, M = prod(m[i])
int64_t crt(const std::vector<int64_t>& r, const std::vector<int64_t>& m);

// Дискретний логарифм алгоритмом Похліга-Гелмана: alpha^x = beta (mod p)
// Повертає x або -1 якщо розв'язку немає
int64_t pohlig_hellman(int64_t alpha, int64_t beta, int64_t p);
