#include <set>
#include <experimental/random>
#include <cinttypes>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <benchmark/benchmark.h>

std::size_t hash_combine(std::size_t left, std::size_t right)
{
	// Shamelessly stolen from boost::hash_combine.
	return left + 0x9e377b9 + (right << 6) + (right >> 2);
}

struct Coordinates : std::pair<int32_t, int32_t>
{
	using std::pair<int32_t, int32_t>::pair;
	const int32_t& x() const { return this->first; }
	const int32_t& y() const { return this->second; }
	int32_t& x() { return this->first; }
	int32_t& y() { return this->second; }
};

template<typename P>
struct int_pair_hash
{
	std::size_t operator()(P const& p) const
	{
		return hash_combine(p.first, p.second);
	}
};

static std::set<Coordinates> ConstructSet(int size)
{
	std::set<Coordinates> s;
	for (int x = 0; x < size; x++)
	{
		s.insert({x, x});
	}
	return s;
}

static std::vector<Coordinates> ConstructVector(int size)
{
	std::vector<Coordinates> v;
	for (int x = 0; x < size; x++)
	{
		v.push_back({x, x});
	}
	return v;
}

static void bench_set_construct(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		auto s = ConstructSet(state.range(0));
		benchmark::ClobberMemory();
	}
}

static void bench_vec_construct(benchmark::State& state)
{
	while (state.KeepRunning())
	{
		auto v= ConstructVector(state.range(0));
		benchmark::ClobberMemory();
	}
}

static void bench_set_find(benchmark::State& state)
{
	auto s = ConstructSet(state.range(0));
	while (state.KeepRunning())
	{
		int to_find = std::experimental::randint(0, state.range(0));
		bool found = true;
		if (s.find({to_find, to_find}) == s.end())
		{
			found = false;
		}
	}
}

static void bench_vec_find(benchmark::State& state)
{
	auto v = ConstructVector(state.range(0));
	while (state.KeepRunning())
	{
		int to_find = std::experimental::randint(0, state.range(0));
		bool found = true;
		if (std::binary_search(v.begin(), v.end(), Coordinates{to_find, to_find}))
		{
			found = false;
		}
	}
}

BENCHMARK(bench_set_construct)->Range(2, 512);
BENCHMARK(bench_vec_construct)->Range(2, 512);
BENCHMARK(bench_set_find)->Range(2, 65536);
BENCHMARK(bench_vec_find)->Range(2, 65536);

BENCHMARK_MAIN();
