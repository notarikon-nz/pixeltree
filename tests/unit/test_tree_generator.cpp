#include <pixeltree/pixeltree.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace pixeltree;

TEST_CASE("TreeGenerator basic functionality", "[generator]") {
    TreeGenerator32 generator(12345); // Fixed seed for reproducible tests
    
    SECTION("Generate basic oak tree") {
        auto params = TreePresets::oak();
        params.canvas_width = 64;
        params.canvas_height = 64;
        
        auto [buffer, metadata] = generator.generate(params);
        
        REQUIRE(buffer.width() == 64);
        REQUIRE(buffer.height() == 64);
        REQUIRE(metadata.branch_count > 0);
        REQUIRE(metadata.generation_time_ms > 0.0f);
    }
    
    SECTION("Generate pine tree") {
        auto params = TreePresets::pine();
        
        auto [buffer, metadata] = generator.generate(params);
        
        REQUIRE(buffer.width() == params.canvas_width.get());
        REQUIRE(buffer.height() == params.canvas_height.get());
        REQUIRE(metadata.branch_count > 0);
    }
    
    SECTION("Dead tree has no leaves") {
        auto params = TreePresets::dead();
        
        auto [buffer, metadata] = generator.generate(params);
        
        REQUIRE(metadata.leaf_count == 0);
    }
}

TEST_CASE("TreeParameters validation", "[parameters]") {
    SECTION("Bounded values stay in range") {
        BoundedFloat01 prob(1.5f); // Should clamp to 1.0
        REQUIRE(prob.get() == 1.0f);
        
        BoundedFloat01 neg_prob(-0.5f); // Should clamp to 0.0
        REQUIRE(neg_prob.get() == 0.0f);
    }
    
    SECTION("Tree parameters validation works") {
        TreeParameters params;
        params.type = TreeType::Pine;
        params.validate();
        
        // Pine trees should have constrained branch angle variation
        REQUIRE(params.branches.branch_angle_variation.get() <= 0.2f);
    }
}

TEST_CASE("PixelBuffer operations", "[pixel_buffer]") {
    SECTION("Basic operations") {
        PixelBuffer32 buffer(10, 10);
        
        REQUIRE(buffer.width() == 10);
        REQUIRE(buffer.height() == 10);
        REQUIRE(buffer.size() == 100);
        
        buffer(5, 5) = 0xFF0000FF; // Red pixel
        REQUIRE(buffer(5, 5) == 0xFF0000FF);
    }
    
    SECTION("Bounds checking") {
        PixelBuffer32 buffer(5, 5);
        
        REQUIRE(buffer.contains(0, 0));
        REQUIRE(buffer.contains(4, 4));
        REQUIRE_FALSE(buffer.contains(5, 5));
        REQUIRE_FALSE(buffer.contains(-1, 0));
    }
}