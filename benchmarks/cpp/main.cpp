
#include <iostream>

#include "benchmark/benchmark.h"

#include "bench.h"

#include "alpaca_bench.h"
#include "blitzbuffers_bench.h"
#include "flatbuffers_bench.h"

static inline void benchmark_encode(benchmark::State& state, Bench& bench) {
	size_t len = 0;

	for (auto _ : state) {
		auto encoded = bench.encode(len);
		benchmark::DoNotOptimize(len);
	}
}

static inline void benchmark_use(benchmark::State& state, Bench& bench) {
	size_t len = 0;

	auto encoded = bench.encode(len);

	size_t result = 0;
	for (auto _ : state) {
		bench.decode(encoded, len);
		result = bench.use(encoded, len);
		benchmark::DoNotOptimize(result);
		benchmark::DoNotOptimize(encoded);
	}
}

static inline void benchmark_roundtrip(benchmark::State& state, Bench& bench) {
	size_t len = 0;

	size_t result = 0;
	for (auto _ : state) {
		auto encoded = bench.encode(len);
		benchmark::DoNotOptimize(len);
		bench.decode(encoded, len);
		result = bench.use(encoded, len);
		benchmark::DoNotOptimize(result);
	}
}


// Encoding
static void blitzbuffers_encode(benchmark::State& state) {
	BlitzBuffersContainer::BlitzBuffersBench bench(blitzbuffers::FixedSizeBufferBackend(1024));
	benchmark_encode(state, bench);
}
BENCHMARK(blitzbuffers_encode);

static void blitzbuffers_expanding_encode(benchmark::State& state) {
	BlitzBuffersContainer::BlitzBuffersBench bench(blitzbuffers::ExpandableBufferBackend(1024));
	benchmark_encode(state, bench);
}
BENCHMARK(blitzbuffers_expanding_encode);

static void alpaca_encode(benchmark::State& state) {
	AlpacaContainer::AlpacaBench bench;
	benchmark_encode(state, bench);
}
BENCHMARK(alpaca_encode);

static void flatbuffers_encode(benchmark::State& state) {
	FlatBuffersContainer::FlatBuffersBench bench;
	benchmark_encode(state, bench);
}
BENCHMARK(flatbuffers_encode);



// Use
static void blitzbuffers_use(benchmark::State& state) {
	BlitzBuffersContainer::BlitzBuffersBench bench(blitzbuffers::FixedSizeBufferBackend(1024));
	benchmark_use(state, bench);
}
BENCHMARK(blitzbuffers_use);

static void blitzbuffers_expanding_use(benchmark::State& state) {
	BlitzBuffersContainer::BlitzBuffersBench bench(blitzbuffers::ExpandableBufferBackend(1024));
	benchmark_use(state, bench);
}
BENCHMARK(blitzbuffers_expanding_use);

static void alpaca_use(benchmark::State& state) {
	AlpacaContainer::AlpacaBench bench;
	benchmark_use(state, bench);
}
BENCHMARK(alpaca_use);

static void flatbuffers_use(benchmark::State& state) {
	FlatBuffersContainer::FlatBuffersBench bench;
	benchmark_use(state, bench);
}
BENCHMARK(flatbuffers_use);


// Roundtrip
static void blitzbuffers_roundtrip(benchmark::State& state) {
	BlitzBuffersContainer::BlitzBuffersBench bench(blitzbuffers::FixedSizeBufferBackend(1024));
	benchmark_roundtrip(state, bench);
}
BENCHMARK(blitzbuffers_roundtrip);

static void blitzbuffers_expanding_roundtrip(benchmark::State& state) {
	BlitzBuffersContainer::BlitzBuffersBench bench(blitzbuffers::ExpandableBufferBackend(1024));
	benchmark_roundtrip(state, bench);
}
BENCHMARK(blitzbuffers_expanding_roundtrip);

static void alpaca_roundtrip(benchmark::State& state) {
	AlpacaContainer::AlpacaBench bench;
	benchmark_roundtrip(state, bench);
}
BENCHMARK(alpaca_roundtrip);

static void flatbuffers_roundtrip(benchmark::State& state) {
	FlatBuffersContainer::FlatBuffersBench bench;
	benchmark_roundtrip(state, bench);
}
BENCHMARK(flatbuffers_roundtrip);



BENCHMARK_MAIN();
