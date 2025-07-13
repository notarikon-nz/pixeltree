#include <pixeltree/pixeltree.hpp>
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <fstream>

using namespace pixeltree;

// Generate a forest of trees with different parameters
void generate_forest_example() {
    std::cout << "Generating procedural forest...\n";
    
    constexpr int forest_size = 100;
    constexpr int canvas_size = 64;
    
    TreeGenerator32 generator;
    std::vector<TreeParameters> tree_params;
    tree_params.reserve(forest_size);
    
    // Create diverse tree parameters
    Random param_rng(42);
    for (int i = 0; i < forest_size; ++i) {
        TreeParameters params;
        
        // Random tree type
        const int type_choice = param_rng.next_int(0, 3);
        switch (type_choice) {
            case 0: params = TreePresets::oak(); break;
            case 1: params = TreePresets::pine(); break;
            case 2: params = TreePresets::palm(); break;
            case 3: params = TreePresets::dead(); break;
        }
        
        // Random variations
        params.canvas_width = canvas_size;
        params.canvas_height = canvas_size;
        params.overall_scale = param_rng.next_float(0.7f, 1.3f);
        params.branches.branch_probability = param_rng.next_float(0.5f, 0.9f);
        params.leaves.density = param_rng.next_float(0.6f, 1.0f);
        
        // Random season
        const int season_choice = param_rng.next_int(0, 3);
        params.season = static_cast<Season>(season_choice);
        params.validate();
        
        tree_params.push_back(params);
    }
    
    // Measure generation time
    const auto start_time = std::chrono::high_resolution_clock::now();
    
    // Generate all trees
    auto results = generator.generate_batch(tree_params);
    
    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    // Calculate statistics
    size_t total_branches = 0;
    size_t total_leaves = 0;
    float total_gen_time = 0.0f;
    
    for (const auto& [buffer, metadata] : results) {
        total_branches += metadata.branch_count;
        total_leaves += metadata.leaf_count;
        total_gen_time += metadata.generation_time_ms;
    }
    
    std::cout << "Forest generation complete!\n";
    std::cout << "Trees generated: " << forest_size << "\n";
    std::cout << "Total branches: " << total_branches << "\n";
    std::cout << "Total leaves: " << total_leaves << "\n";
    std::cout << "Total generation time: " << duration.count() << "ms\n";
    std::cout << "Average per tree: " << (duration.count() / float(forest_size)) << "ms\n";
    std::cout << "Trees per second: " << (forest_size * 1000.0f / duration.count()) << "\n";
}

// Demonstrate async generation
void async_generation_example() {
    std::cout << "\nAsync generation example...\n";
    
    TreeGenerator32 generator;
    
    // Start multiple async generations
    std::vector<std::future<std::pair<PixelBuffer32, TreeMetadata>>> futures;
    
    const std::vector<TreeParameters> params_list = {
        TreePresets::oak(),
        TreePresets::pine(),
        TreePresets::palm(),
        TreePresets::dead()
    };
    
    for (const auto& params : params_list) {
        futures.emplace_back(generator.generate_async(params));
    }
    
    // Collect results as they complete
    for (size_t i = 0; i < futures.size(); ++i) {
        auto [buffer, metadata] = futures[i].get();
        std::cout << "Tree " << i << " completed: " 
                  << metadata.branch_count << " branches, "
                  << metadata.generation_time_ms << "ms\n";
    }
}

// Performance benchmarking
void benchmark_generation() {
    std::cout << "\nPerformance benchmark...\n";
    
    TreeGenerator32 generator;
    auto params = TreePresets::oak();
    
    // Warm up
    for (int i = 0; i < 10; ++i) {
        generator.generate(params);
    }
    
    // Benchmark different canvas sizes
    const std::vector<int> sizes = {32, 64, 128, 256};
    
    for (int size : sizes) {
        params.canvas_width = size;
        params.canvas_height = size;
        
        const auto start = std::chrono::high_resolution_clock::now();
        
        constexpr int iterations = 50;
        for (int i = 0; i < iterations; ++i) {
            generator.generate(params);
        }
        
        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start);
        
        const float avg_time = duration.count() / float(iterations) / 1000.0f; // ms
        
        std::cout << "Canvas " << size << "x" << size 
                  << ": " << avg_time << "ms per tree\n";
    }
}

int main() {
    std::cout << "PixelTree Advanced Examples\n";
    std::cout << "=============================\n";
    std::cout << "Library version: " << pixeltree::version_string() << "\n";
    std::cout << "SIMD support: " << (pixeltree::has_simd_support() ? "Yes" : "No") << "\n";
    std::cout << "OpenMP support: " << (pixeltree::has_openmp_support() ? "Yes" : "No") << "\n\n";
    
    try {
        generate_forest_example();
        async_generation_example();
        benchmark_generation();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}