#include <benchmark/benchmark.h>
#include <string>
#include <vector>

// --- A: 按值传参 + move
struct FieldA {
    std::string key;
    std::string name;
    FieldA(std::string k, std::string n)
            : key(std::move(k)), name(std::move(n)) {}
};

// --- B: 按 const& 传参 + copy
struct FieldB {
    std::string key;
    std::string name;
    FieldB(const std::string& k, const std::string& n)
            : key(k), name(n) {}
};

// --- Benchmark A: 右值构造
static void BM_FieldA_rvalue(benchmark::State& state) {
    for (auto _ : state) {
        FieldA f("hello", "world");  // 右值 → 移动
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(BM_FieldA_rvalue);

static void BM_FieldB_rvalue(benchmark::State& state) {
    for (auto _ : state) {
        FieldB f("hello", "world");  // 右值 → 只能拷贝
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(BM_FieldB_rvalue);

// --- Benchmark B: 左值构造
static void BM_FieldA_lvalue(benchmark::State& state) {
    std::string k = "hello";
    std::string n = "world";
    for (auto _ : state) {
        FieldA f(k, n);  // 左值 → 先 copy 一份参数，再 move 给成员
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(BM_FieldA_lvalue);

static void BM_FieldB_lvalue(benchmark::State& state) {
    std::string k = "hello";
    std::string n = "world";
    for (auto _ : state) {
        FieldB f(k, n);  // 左值 → 直接 copy 给成员
        benchmark::DoNotOptimize(f);
    }
}
BENCHMARK(BM_FieldB_lvalue);

BENCHMARK_MAIN();
