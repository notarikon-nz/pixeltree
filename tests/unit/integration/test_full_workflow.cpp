#include <pixeltree/pixeltree.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <filesystem>

using namespace pixeltree;

TEST_CASE("Full tree generation workflow", "[integration]") {
    TreeGenerator32 generator;
    
    SECTION("Generate and validate all tree types") {
        const std::vector<std::pair<std::string, TreeParameters>> tree_types = {
            {"oak", TreePresets::oak()},
            {"pine", TreePresets::pine()},
            {"palm", TreePresets::palm()},
            {"dead", TreePresets::dead()}
        };
        
        for (const auto& [name, params] : tree_types) {
            CAPTURE(name);
            
            auto [buffer, metadata] = generator.generate(params);
            
            // Validate buffer
            REQUIRE(buffer.width() == params.canvas_width.get());
            REQUIRE(buffer.height() == params.canvas_height.get());
            REQUIRE(!buffer.empty());
            
            // Validate metadata
            REQUIRE(metadata.branch_count > 0);
            REQUIRE(metadata.generation_time_ms > 0.0f);
            REQUIRE(metadata.max_depth > 0);
            
            // Tree-specific validations
            if (params.type == TreeType::Dead) {
                REQUIRE(metadata.leaf_count == 0);
            } else {
                REQUIRE(metadata.leaf_count > 0);
            }
        }
    }
    
    SECTION("Seasonal variations work correctly") {
        auto params = TreePresets::oak();
        
        for (int s = 0; s < 4; ++s) {
            params.season = static_cast<Season>(s);
            params.validate();
            
            auto [buffer, metadata] = generator.generate(params);
            
            REQUIRE(buffer.width() == params.canvas_width.get());
            REQUIRE(metadata.branch_count > 0);
            
            // Winter should have fewer leaves
            if (params.season == Season::Winter) {
                REQUIRE(metadata.leaf_count < 10); // Fewer leaves in winter
            }
        }
    }
    
    SECTION("Batch generation consistency") {
        std::vector<TreeParameters> params_list;
        for (int i = 0; i < 10; ++i) {
            auto params = TreePresets::oak();
            params.random_seed = 1000 + i; // Different seeds
            params_list.push_back(params);
        }
        
        auto results = generator.generate_batch(params_list);
        
        REQUIRE(results.size() == 10);
        
        for (const auto& [buffer, metadata] : results) {
            REQUIRE(buffer.width() > 0);
            REQUIRE(buffer.height() > 0);
            REQUIRE(metadata.branch_count > 0);
        }
    }
    
    SECTION("Memory management stress test") {
        // Generate many trees to test memory management
        constexpr int stress_count = 1000;
        
        for (int i = 0; i < stress_count; ++i) {
            auto params = TreePresets::oak();
            params.random_seed = i;
            
            auto [buffer, metadata] = generator.generate(params);
            
            // Just verify basic validity
            REQUIRE(buffer.width() > 0);
            REQUIRE(metadata.branch_count > 0);
            
            // Buffer should be automatically cleaned up here
        }
    }
}

TEST_CASE("Cross-platform compatibility", "[integration]") {
    SECTION("Deterministic generation across platforms") {
        TreeGenerator32 generator1(12345);
        TreeGenerator32 generator2(12345);
        
        auto params = TreePresets::oak();
        
        auto [buffer1, metadata1] = generator1.generate(params);
        auto [buffer2, metadata2] = generator2.generate(params);
        
        // Same seed should produce identical results
        REQUIRE(metadata1.branch_count == metadata2.branch_count);
        REQUIRE(metadata1.leaf_count == metadata2.leaf_count);
        REQUIRE(metadata1.max_depth == metadata2.max_depth);
        
        // Pixel data should be identical
        REQUIRE(buffer1.width() == buffer2.width());
        REQUIRE(buffer1.height() == buffer2.height());
        
        bool pixels_match = true;
        for (size_t i = 0; i < buffer1.size(); ++i) {
            if (buffer1.data()[i] != buffer2.data()[i]) {
                pixels_match = false;
                break;
            }
        }
        REQUIRE(pixels_match);
    }
}