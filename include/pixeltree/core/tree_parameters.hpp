#pragma once
#include "math_types.hpp"
#include <array>
#include <vector>
#include <optional>

namespace pixeltree {

// Tree type enumeration
enum class TreeType : uint8_t {
    Oak = 0,
    Pine,
    Palm,
    Birch,
    Willow,
    Dead,
    Custom
};

// Growth stage enumeration
enum class GrowthStage : uint8_t {
    Seed = 0,
    Sapling,
    Young,
    Mature,
    Old,
    Dead
};

// Season enumeration for color variations
enum class Season : uint8_t {
    Spring = 0,
    Summer,
    Autumn,
    Winter
};

// Branch generation parameters
struct BranchParameters {
    BoundedFloat10 base_thickness{2.0f};        // Base trunk thickness
    BoundedFloat01 thickness_decay{0.8f};       // Thickness reduction per level
    BoundedFloat01 branch_probability{0.7f};    // Probability of branching
    BoundedFloat01 branch_angle_variation{0.3f}; // Angle variation (0-1)
    BoundedValue<int, 1, 10> max_depth{5};      // Maximum branch depth
    BoundedValue<int, 8, 64> max_branches{32};  // Maximum total branches
    BoundedFloat01 curvature{0.1f};             // Branch curvature amount
    BoundedFloat01 asymmetry{0.2f};             // Left/right asymmetry
};

// Leaf generation parameters
struct LeafParameters {
    BoundedFloat01 density{0.8f};               // Leaf density at branch tips
    BoundedFloat10 size_base{3.0f};             // Base leaf cluster size
    BoundedFloat01 size_variation{0.3f};        // Size variation (0-1)
    BoundedFloat01 color_variation{0.2f};       // Color variation amount
    std::array<Color, 4> base_colors{          // Base colors for blending
        Color{34, 139, 34},   // Forest green
        Color{50, 205, 50},   // Lime green  
        Color{107, 142, 35},  // Olive drab
        Color{85, 107, 47}    // Dark olive
    };
    BoundedFloat01 alpha_variation{0.1f};       // Alpha transparency variation
};

// Trunk appearance parameters
struct TrunkParameters {
    Color base_color{101, 67, 33};              // Base trunk color
    BoundedFloat01 color_variation{0.15f};      // Color variation amount
    BoundedFloat01 texture_noise{0.1f};         // Texture noise intensity
    BoundedFloat01 bark_detail{0.0f};           // Bark texture detail (0 = smooth)
};

// Main tree generation parameters
struct TreeParameters {
    // Basic properties
    TreeType type = TreeType::Oak;
    GrowthStage growth_stage = GrowthStage::Mature;
    Season season = Season::Summer;
    
    // Size constraints
    BoundedValue<int, 16, 512> canvas_width{128};
    BoundedValue<int, 16, 512> canvas_height{128};
    BoundedFloat10 overall_scale{1.0f};
    
    // Component parameters
    BranchParameters branches;
    LeafParameters leaves;
    TrunkParameters trunk;
    
    // Environmental factors
    BoundedAngle wind_direction{0.0f};          // Wind direction in degrees
    BoundedFloat01 wind_strength{0.0f};         // Wind strength (affects lean)
    BoundedFloat01 age_factor{0.5f};            // Age factor (affects appearance)
    
    // Generation settings
    uint32_t random_seed = 0;                   // 0 = use random seed
    BoundedFloat01 determinism{0.8f};           // How deterministic vs random
    
    // Validation and normalization
    void validate() noexcept {
        // Adjust parameters based on tree type
        switch (type) {
            case TreeType::Pine:
                branches.branch_angle_variation = std::min(branches.branch_angle_variation.get(), 0.2f);
                leaves.density = std::min(leaves.density.get(), 0.6f);
                break;
            case TreeType::Palm:
                branches.max_depth = std::min(branches.max_depth.get(), 3);
                branches.curvature = std::max(branches.curvature.get(), 0.3f);
                break;
            case TreeType::Willow:
                branches.curvature = std::max(branches.curvature.get(), 0.4f);
                break;
            case TreeType::Dead:
                leaves.density = 0.0f;
                trunk.color_variation = std::max(trunk.color_variation.get(), 0.3f);
                break;
            default:
                break;
        }
        
        // Adjust for growth stage
        const float growth_factor = static_cast<float>(growth_stage) / static_cast<float>(GrowthStage::Mature);
        overall_scale = overall_scale.get() * (0.2f + 0.8f * growth_factor);
        
        // Adjust for season
        if (season == Season::Autumn) {
            // Autumn colors
            leaves.base_colors[0] = Color{255, 140, 0};   // Dark orange
            leaves.base_colors[1] = Color{255, 165, 0};   // Orange
            leaves.base_colors[2] = Color{255, 69, 0};    // Red orange
            leaves.base_colors[3] = Color{139, 69, 19};   // Saddle brown
        } else if (season == Season::Winter) {
            leaves.density = leaves.density.get() * 0.3f; // Sparse leaves
        }
    }
};

// Preset configurations for common tree types
class TreePresets {
public:
    static TreeParameters oak() {
        TreeParameters params;
        params.type = TreeType::Oak;
        params.branches.branch_probability = 0.8f;
        params.branches.max_depth = 5;
        params.branches.curvature = 0.15f;
        params.leaves.density = 0.9f;
        params.leaves.size_base = 4.0f;
        return params;
    }
    
    static TreeParameters pine() {
        TreeParameters params;
        params.type = TreeType::Pine;
        params.branches.branch_probability = 0.6f;
        params.branches.branch_angle_variation = 0.15f;
        params.branches.max_depth = 6;
        params.leaves.density = 0.7f;
        params.leaves.base_colors[0] = Color{34, 139, 34};   // Forest green
        params.leaves.base_colors[1] = Color{0, 100, 0};     // Dark green
        params.leaves.base_colors[2] = Color{46, 125, 50};   // Medium green
        params.leaves.base_colors[3] = Color{27, 94, 32};    // Dark forest green
        return params;
    }
    
    static TreeParameters palm() {
        TreeParameters params;
        params.type = TreeType::Palm;
        params.branches.max_depth = 2;
        params.branches.curvature = 0.4f;
        params.branches.base_thickness = 1.5f;
        params.leaves.density = 0.4f;
        params.leaves.size_base = 8.0f;
        return params;
    }
    
    static TreeParameters dead() {
        TreeParameters params;
        params.type = TreeType::Dead;
        params.branches.branch_probability = 0.5f;
        params.leaves.density = 0.0f;
        params.trunk.base_color = Color{101, 67, 33};
        params.trunk.color_variation = 0.4f;
        return params;
    }
};

} // namespace pixeltree