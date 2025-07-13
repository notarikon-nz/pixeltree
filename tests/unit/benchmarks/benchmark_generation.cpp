#include <pixeltree/pixeltree.hpp>
#include <benchmark/benchmark.h>

using namespace pixeltree;

// Benchmark basic tree generation
static void BM_TreeGeneration_Oak(benchmark::State& state) {
    TreeGenerator32 generator(12345);
    auto params = TreePresets::oak();
    params.canvas_width = state.range(0);
    params.canvas_height = state.range(0);
    
    for (auto _ : state) {
        auto [buffer, metadata] = generator.generate(params);
        benchmark::DoNotOptimize(buffer);
    }
    
    state.SetComplexityN(state.range(0) * state.range(0));
}
BENCHMARK(BM_TreeGeneration_Oak)
    ->Range(32, 256)
    ->Complexity(benchmark::oN);

// Benchmark batch generation
static void BM_BatchGeneration(benchmark::State& state) {
    TreeGenerator32 generator(12345);
    
    std::vector<TreeParameters> params_list;
    for (int i = 0; i < state.range(0); ++i) {
        auto params = TreePresets::oak();
        params.canvas_width = 64;
        params.canvas_height = 64;
        params_list.push_back(params);
    }
    
    for (auto _ : state) {
        auto results = generator.generate_batch(params_list);
        benchmark::DoNotOptimize(results);
    }
    
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_BatchGeneration)
    ->Range(1, 100)
    ->Unit(benchmark::kMillisecond);

// Benchmark pixel buffer operations
static void BM_PixelBuffer_Clear(benchmark::State& state) {
    PixelBuffer32 buffer(state.range(0), state.range(0));
    
    for (auto _ : state) {
        buffer.clear(0xFF0000FF);
        benchmark::DoNotOptimize(buffer);
    }
    
    state.SetBytesProcessed(state.iterations() * buffer.size() * sizeof(uint32_t));
}
BENCHMARK(BM_PixelBuffer_Clear)
    ->Range(64, 512);

BENCHMARK_MAIN();