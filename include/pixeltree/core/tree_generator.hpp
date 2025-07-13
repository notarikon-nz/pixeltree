#pragma once
#include "tree_parameters.hpp"
#include "tree_structure.hpp"
#include "pixel_buffer.hpp"
#include "tree_renderer.hpp"
#include "lsystem.hpp"
#include "random.hpp"
#include <memory>
#include <future>

namespace pixeltree {

// Tree generation metadata
struct TreeMetadata {
    uint32_t generation_id;
    size_t branch_count;
    size_t leaf_count;
    int max_depth;
    float generation_time_ms;
    Rect2Df bounding_box;
    uint32_t random_seed;
};

// Main tree generator class
template<typename PixelType = uint32_t, size_t MaxBranches = 64>
class TreeGenerator {
    mutable Random rng_;
    LSystemGenerator lsystem_;
    TreeRenderer renderer_;
    
public:
    static constexpr size_t max_branches = MaxBranches;
    
    // Constructor
    explicit TreeGenerator(uint32_t seed = 0) 
        : rng_(seed == 0 ? std::random_device{}() : seed) {
    }
    
    // Generate tree and return both structure and rendered buffer
    auto generate(const TreeParameters& params) 
        -> std::pair<PixelBuffer<PixelType>, TreeMetadata> {
        
        const auto start_time = std::chrono::high_resolution_clock::now();
        
        // Setup random seed
        const uint32_t actual_seed = params.random_seed == 0 ? 
            std::random_device{}() : params.random_seed;
        rng_ = Random(actual_seed);
        
        // Validate and normalize parameters
        TreeParameters normalized_params = params;
        normalized_params.validate();
        
        // Setup L-System rules for tree type
        lsystem_.setup_rules(normalized_params.type);
        
        // Generate L-System string
        const std::string lstring = lsystem_.generate_string(normalized_params, rng_);
        
        // Convert to tree structure
        auto tree_structure = lsystem_.string_to_tree(lstring, normalized_params, rng_);
        
        // Generate leaf clusters
        generate_leaf_clusters(*tree_structure, rng_);
        
        // Calculate bounding box
        tree_structure->calculate_bounding_box();
        
        // Render to pixel buffer
        PixelBuffer<PixelType> pixel_buffer;
        if constexpr (std::is_same_v<PixelType, uint32_t>) {
            pixel_buffer = std::move(renderer_.render(*tree_structure));
        } else {
            // Convert from uint32_t to other pixel types
            auto rgba_buffer = renderer_.render(*tree_structure);
            pixel_buffer = convert_pixel_buffer<PixelType>(rgba_buffer);
        }
        
        // Calculate generation time
        const auto end_time = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);
        const float generation_time = duration.count() / 1000.0f; // Convert to milliseconds
        
        // Create metadata
        TreeMetadata metadata{
            .generation_id = tree_structure->generation_id,
            .branch_count = tree_structure->branch_count(),
            .leaf_count = tree_structure->leaf_cluster_count(),
            .max_depth = tree_structure->max_depth(),
            .generation_time_ms = generation_time,
            .bounding_box = tree_structure->bounding_box,
            .random_seed = actual_seed
        };
        
        return {std::move(pixel_buffer), metadata};
    }
    
    // Generate only tree structure (without rendering)
    std::unique_ptr<TreeStructure> generate_structure(const TreeParameters& params) {
        // Setup random seed
        const uint32_t actual_seed = params.random_seed == 0 ? 
            std::random_device{}() : params.random_seed;
        rng_ = Random(actual_seed);
        
        TreeParameters normalized_params = params;
        normalized_params.validate();
        
        lsystem_.setup_rules(normalized_params.type);
        const std::string lstring = lsystem_.generate_string(normalized_params, rng_);
        auto tree_structure = lsystem_.string_to_tree(lstring, normalized_params, rng_);
        
        generate_leaf_clusters(*tree_structure, rng_);
        tree_structure->calculate_bounding_box();
        
        return tree_structure;
    }
    
    // Render existing tree structure
    PixelBuffer<PixelType> render_structure(const TreeStructure& tree) const {
        if constexpr (std::is_same_v<PixelType, uint32_t>) {
            return renderer_.render(tree);
        } else {
            auto rgba_buffer = renderer_.render(tree);
            return convert_pixel_buffer<PixelType>(rgba_buffer);
        }
    }
    
    // Batch generation for multiple trees
    std::vector<std::pair<PixelBuffer<PixelType>, TreeMetadata>> 
    generate_batch(const std::vector<TreeParameters>& params_list) {
        std::vector<std::pair<PixelBuffer<PixelType>, TreeMetadata>> results;
        results.reserve(params_list.size());
        
        for (const auto& params : params_list) {
            results.emplace_back(generate(params));
        }
        
        return results;
    }
    
    // Async generation
    std::future<std::pair<PixelBuffer<PixelType>, TreeMetadata>> 
    generate_async(const TreeParameters& params) {
        return std::async(std::launch::async, [this, params]() {
            return generate(params);
        });
    }

private:
    // Generate leaf clusters at branch endpoints
    void generate_leaf_clusters(TreeStructure& tree, Random& rng) const {
        if (tree.parameters.leaves.density.get() <= 0.0f) {
            return; // No leaves for dead trees
        }
        
        const auto leaf_branches = tree.get_leaf_branches();
        tree.leaf_clusters.reserve(leaf_branches.size());
        
        for (const auto& branch : leaf_branches) {
            if (rng.next_float() < tree.parameters.leaves.density.get()) {
                // Create leaf cluster at branch endpoint
                const float base_size = tree.parameters.leaves.size_base.get();
                const float size_var = tree.parameters.leaves.size_variation.get();
                const float cluster_size = base_size * (1.0f + rng.next_float(-size_var, size_var));
                
                // Color variation
                const auto& base_colors = tree.parameters.leaves.base_colors;
                const Color base_color = base_colors[rng.next_int(0, 3)];
                const float color_var = tree.parameters.leaves.color_variation.get();
                
                Color leaf_color{
                    static_cast<uint8_t>(std::clamp(base_color.r * (1.0f + rng.next_float(-color_var, color_var)), 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(base_color.g * (1.0f + rng.next_float(-color_var, color_var)), 0.0f, 255.0f)),
                    static_cast<uint8_t>(std::clamp(base_color.b * (1.0f + rng.next_float(-color_var, color_var)), 0.0f, 255.0f)),
                    base_color.a
                };
                
                LeafCluster cluster(branch->end_point, cluster_size, leaf_color);
                
                // Set cluster shape based on tree type
                switch (tree.parameters.type) {
                    case TreeType::Pine:
                        cluster.shape = LeafCluster::Shape::Spiky;
                        break;
                    case TreeType::Palm:
                        cluster.shape = LeafCluster::Shape::Ellipse;
                        break;
                    case TreeType::Willow:
                        cluster.shape = LeafCluster::Shape::Scattered;
                        break;
                    default:
                        cluster.shape = LeafCluster::Shape::Circle;
                        break;
                }
                
                tree.leaf_clusters.emplace_back(std::move(cluster));
            }
        }
    }
    
    // Convert pixel buffer types
    template<typename TargetPixelType>
    PixelBuffer<TargetPixelType> convert_pixel_buffer(const PixelBuffer32& source) const {
        PixelBuffer<TargetPixelType> result(source.width(), source.height());
        
        for (size_t i = 0; i < source.size(); ++i) {
            if constexpr (std::is_same_v<TargetPixelType, uint8_t>) {
                // Convert RGBA to grayscale
                const uint32_t rgba = source.data()[i];
                const uint8_t r = (rgba >> 24) & 0xFF;
                const uint8_t g = (rgba >> 16) & 0xFF;
                const uint8_t b = (rgba >> 8) & 0xFF;
                result.data()[i] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
            } else {
                result.data()[i] = static_cast<TargetPixelType>(source.data()[i]);
            }
        }
        
        return result;
    }
};

// Convenience type aliases
using TreeGenerator32 = TreeGenerator<uint32_t, 64>;
using TreeGenerator8 = TreeGenerator<uint8_t, 32>;

} // namespace pixeltree