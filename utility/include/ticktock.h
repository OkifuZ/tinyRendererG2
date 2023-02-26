#pragma once

#include <iostream>
#include <chrono>


#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();
#define TOCK_OUT(x) printf("%s: %lfs\n", #x, std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - bench_##x).count());
#define TOCK_VAL(x) (std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - bench_##x).count())
#define TOCK_VALF(x) (std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - bench_##x).count())