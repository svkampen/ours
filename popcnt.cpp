#include <x86intrin.h>
#include <benchmark/benchmark.h>

static void bench_popcnt(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        volatile int a;
        benchmark::DoNotOptimize(a = _mm_popcnt_u32(state.range(0)));
    }
}

static void bench_normal(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        volatile int count = 0;
        volatile int a = state.range(0);
        while (a)
        {
            benchmark::DoNotOptimize(count++);
            benchmark::DoNotOptimize(a &= (a - 1));
        }
    }
}

BENCHMARK(bench_popcnt)->Range(1, 65536*128);
BENCHMARK(bench_normal)->Range(1, 65536*128);

BENCHMARK_MAIN();
