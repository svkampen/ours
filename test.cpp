#include <unordered_map>
#include <simmo/point.hpp>
#include <benchmark/benchmark.h>
#include <random>
#include <cinttypes>
#include <map>

struct Coordinates : std::pair<int32_t, int32_t>
{
	using std::pair<int32_t, int32_t>::pair;
};

using ChunkMap = std::map<Coordinates, int>;
using SimmoChunkMap = std::unordered_map<simmo::point<int32_t, 2>, int>;

size_t hash_combine(size_t left, size_t right)
{
	return left + 0x9e3779b9 + (right << 6) + (right >> 2);
}

struct hash_pair
{
	size_t operator()(Coordinates const& p) const
	{
		return hash_combine(p.first, p.second);
	}
};

using ChunkHashMap = std::unordered_map<Coordinates, int, hash_pair>;

template<typename T>
static T ConstructMap(int xymax)
{
	T t;
	for (int x = 0; x < xymax; x++)
	{
		for (int y = 0; y < xymax; y++)
		{
			t[{x, y}] = 42;
		}
	}
	return t;
}

static void bench_hashpair_find_chunk(benchmark::State& state) {
	std::random_device r;
	std::default_random_engine eng(r());
	double dist_size = 1 + (state.range(1) / 100.0);
	std::uniform_int_distribution<int> uniform_dist(0, (int)(dist_size*state.range(0)));
	ChunkHashMap m = ConstructMap<ChunkHashMap>(state.range(0));

	while (state.KeepRunning())
	{
		volatile int v;

		Coordinates coordinates = {uniform_dist(eng), uniform_dist(eng)};
		auto chunk_iter = m.find(coordinates);
		if (chunk_iter == m.end())
			v = 0;
		else
			v = m[coordinates];
	}
}

static void bench_pair_find_chunk(benchmark::State& state) {
	std::random_device r;
	std::default_random_engine eng(r());
	double dist_size = 1 + (state.range(1) / 100.0);
	std::uniform_int_distribution<int> uniform_dist(0, (int)(dist_size*state.range(0)));
	ChunkMap m = ConstructMap<ChunkMap>(state.range(0));

	while (state.KeepRunning())
	{
		volatile int v;

		Coordinates coordinates = {uniform_dist(eng), uniform_dist(eng)};
		auto chunk_iter = m.find(coordinates);
		if (chunk_iter == m.end())
			v = 0;
		else
			v = m[coordinates];
	}
}

static void bench_sim_find_chunk(benchmark::State& state)
{
	std::random_device r;
	std::default_random_engine eng(r());
	double dist_size = 1 + (state.range(1) / 100.0);
	std::uniform_int_distribution<int> uniform_dist(0, (int)(dist_size*state.range(0)));
	SimmoChunkMap m = ConstructMap<SimmoChunkMap>(state.range(0));

	while (state.KeepRunning())
	{
		volatile int v;
		simmo::point<int32_t, 2> coordinates = {uniform_dist(eng), uniform_dist(eng)};
		auto chunk_iter = m.find(coordinates);
		if (chunk_iter == m.end())
			v = 0;
		else
			v = m[coordinates];
	}
}

static void CustomArgs(benchmark::internal::Benchmark* b)
{
	for (int i = 8; i <= 256; i *= 2)
		for (int j = 20; j <= 80; j += 20)
			b->ArgPair(i, j);
}

BENCHMARK(bench_pair_find_chunk)->Apply(CustomArgs);
BENCHMARK(bench_hashpair_find_chunk)->Apply(CustomArgs);
BENCHMARK(bench_sim_find_chunk)->Apply(CustomArgs);

BENCHMARK_MAIN();
