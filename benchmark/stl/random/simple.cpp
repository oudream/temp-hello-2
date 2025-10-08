#include <benchmark/benchmark.h>
#include <vector>
#include <numeric>
#include <random>

// 一个简单冒烟基准：求和固定向量
static void BM_Smoke_SumVector(benchmark::State& state) {
    std::vector<int> v(100000, 1);
    for (auto _ : state) {
        auto s = std::accumulate(v.begin(), v.end(), 0);
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_Smoke_SumVector)->Name("Smoke/SumVector");

// 参数化：不同 N 下的 push_back
static void BM_PushBack(benchmark::State& state) {
    const int N = static_cast<int>(state.range(0));
    for (auto _ : state) {
        std::vector<int> v;
        v.reserve(N);
        for (int i = 0; i < N; ++i) v.push_back(i);
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_PushBack)->Arg(1 << 10)->Arg(1 << 15)->Arg(1 << 20);

// 注意：如果链接了 benchmark_main，就不需要自定义 main()
