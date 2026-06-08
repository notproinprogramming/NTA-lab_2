#pragma once

#include <cstdint>

// Дискретний логарифм перебором
// Повертає x або -1 при таймауті / відсутності розв'язку
int64_t brute_force_dlog(int64_t alpha, int64_t beta, int64_t p, int timeout_sec = 300);

// Піднесення до степеня за модулем
int64_t mod_pow(int64_t base, int64_t exp, int64_t mod);
