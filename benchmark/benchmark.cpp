#include <benchmark/benchmark.h>
#include "BinaryArithmetic.h"
#include "FactorialArithmetic.h"
#include <random>
#include <string>

using bint = numsystem::BinaryArithmetic;
using fint = numsystem::FactorialArithmetic;

static std::string generate_large_number_string(size_t num_digits) {
    if (num_digits == 0) return "0";
    std::string s(num_digits, ' ');
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> digit_dist(0, 9);

    for (size_t i = 0; i < num_digits; ++i) {
        s[i] = (i == 0 && num_digits > 1) ? '1' + digit_dist(gen) % 9 : '0' + digit_dist(gen);
    }
    return s;
}

// Пример для операций: Add, Sub, Mul, Div, Mod — по отдельности
template <typename T>
static void BM_Add(benchmark::State& state) {
    const size_t n = state.range(0);
    T a(generate_large_number_string(n));
    T b(generate_large_number_string(n));
    for (auto _ : state) benchmark::DoNotOptimize(a + b);
    state.SetComplexityN(n);
}

template <typename T>
static void BM_Sub(benchmark::State& state) {
    const size_t n = state.range(0);
    T a(generate_large_number_string(n));
    T b(generate_large_number_string(n));
    for (auto _ : state) benchmark::DoNotOptimize(a - b);
    state.SetComplexityN(n);
}

template <typename T>
static void BM_Mul(benchmark::State& state) {
    const size_t n = state.range(0);
    T a(generate_large_number_string(n));
    T b(generate_large_number_string(n));
    for (auto _ : state) benchmark::DoNotOptimize(a * b);
    state.SetComplexityN(n);
}

template <typename T>
static void BM_Div(benchmark::State& state) {
    const size_t n = state.range(0);
    T a(generate_large_number_string(n));
    T b(generate_large_number_string(n));
    if (b == T(0)) b = T("1");
    for (auto _ : state) benchmark::DoNotOptimize(a / b);
    state.SetComplexityN(n);
}

template <typename T>
static void BM_Mod(benchmark::State& state) {
    const size_t n = state.range(0);
    T a(generate_large_number_string(n));
    T b(generate_large_number_string(n));
    if (b == T(0)) b = T("1");
    for (auto _ : state) benchmark::DoNotOptimize(a % b);
    state.SetComplexityN(n);
}

template <typename T>
static void BM_Comparison(benchmark::State& state) {
    const size_t n = state.range(0);
    T a(generate_large_number_string(n));
    T b(generate_large_number_string(n));
    for (auto _ : state) {
        benchmark::DoNotOptimize(a < b);
        benchmark::DoNotOptimize(a == b);
        benchmark::DoNotOptimize(a > b);
    }
    state.SetComplexityN(n);
}


#define REGISTER_OP(Type, Func, NameStr, Min, Max) \
    BENCHMARK_TEMPLATE(Func, Type)->Name(NameStr)->RangeMultiplier(2)->Range(Min, Max)->MinTime(0.01)->Complexity()

constexpr int MIN_RANGE_BENCHMARK = 10;
constexpr int MAX_RANGE_BENCHMARK = 500;

// Binary arithmetic
REGISTER_OP(bint, BM_Add, "Binary-Add", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(bint, BM_Sub, "Binary-Sub", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(bint, BM_Mul, "Binary-Mul", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(bint, BM_Div, "Binary-Div", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(bint, BM_Mod, "Binary-Mod", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(bint, BM_Comparison, "Binary-Comparison", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);

// Factorial arithmetic
REGISTER_OP(fint, BM_Add, "Factorial-Add", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(fint, BM_Sub, "Factorial-Sub", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(fint, BM_Mul, "Factorial-Mul", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(fint, BM_Div, "Factorial-Div", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(fint, BM_Mod, "Factorial-Mod", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);
REGISTER_OP(fint, BM_Comparison, "Factorial-Comparison", MIN_RANGE_BENCHMARK, MAX_RANGE_BENCHMARK);


