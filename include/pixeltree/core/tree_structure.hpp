#pragma once
#include "math_types.hpp"
#include "tree_parameters.hpp"
#include "random.hpp"
#include <vector>
#include <memory>
#include <optional>

namespace pixeltree {

// Forward declarations
struct Branch;
struct LeafCluster;

// Branch node in the tree structure
struct Branch {
    // Hierarchy
    std::shared_ptr<Branch> parent;
    std::vector<std::shared_ptr<Branch>> children;
    
    // Geometry
    Point2Df start_point;
    Point2Df end_point;
    float thickness;
    float curvature;
    
    // Visual properties
    Color color;
    int depth_level;
    
    // Generation metadata
    uint32_t generation_id;
    float growth_factor;
    
    // Constructor
    Branch(Point2Df start, Point2Df end, float thick, int depth = 0) 
        : start_point(start), end_point(end), thickness(thick), 
          curvature(0.0f), color(101, 67, 33), depth_level(depth),
          generation_id(0), growth_factor(1.0f) {}
    
    // Utility methods
    Point2Df direction() const noexcept {
        return (end_point - start_point).normalized();
    }
    
    float length() const noexcept {
        return (end_point - start_point).length();
    }
    
    Point2Df midpoint() const noexcept {
        return (start_point + end_point) * 0.5f;
    }
    
    // Check if this is a leaf branch (no children)
    bool is_leaf() const noexcept {
        return children.empty();
    }
    
    // Get all leaf branches in this subtree
    void get_leaf_branches(std::vector<std::shared_ptr<Branch>>& leaves) const {
        if (is_leaf()) {
            leaves.push_back(std::const_pointer_cast<Branch>(shared_from_this()));
        } else {
            for (const auto& child : children) {
                child->get_leaf_branches(leaves);
            }
        }
    }
    
    // Calculate bounding box of this branch
    Rect2Df bounding_box() const noexcept {
        const float half_thickness = thickness * 0.5f;
        return Rect2Df{
            {std::min(start_point.x, end_point.x) - half_thickness,
             std::min(start_point.y, end_point.y) - half_thickness},
            {std::max(start_point.x, end_point.x) + half_thickness,
             std::max(start_point.y, end_point.y) + half_thickness}
        };
    }
    
private:
    // Enable shared_from_this
    std::shared_ptr<Branch> shared_from_this() const {
        return std::const_pointer_cast<Branch>(
            std::static_pointer_cast<const Branch>(
                std::enable_shared_from_this<Branch>::shared_from_this()
            )
        );
    }
};

// Leaf cluster attached to branch endpoints
struct LeafCluster {
    Point2Df position;
    float size;
    Color color;
    std::vector<Point2Df> leaf_positions;  // Individual leaf positions
    
    // Shape information
    enum class Shape {
        Circle,
        Ellipse,
        Spiky,
        Scattered
    } shape = Shape::Circle;
    
    // Constructor
    LeafCluster(Point2Df pos, float sz, Color col) 
        : position(pos), size(sz), color(col) {}
    
    // Generate leaf positions based on shape
    void generate_leaves(Random& rng, int leaf_count) {
        leaf_positions.clear();
        leaf_positions.reserve(leaf_count);
        
        for (int i = 0; i < leaf_count; ++i) {
            Point2Df leaf_pos;
            
            switch (shape) {
                case Shape::Circle:
                    leaf_pos = rng.next_point_in_circle(size);
                    break;
                case Shape::Ellipse:
                    leaf_pos = rng.next_point_in_circle(size);
                    leaf_pos.x *= 1.5f; // Stretch horizontally
                    break;
                case Shape::Spiky:
                    leaf_pos = rng.next_point_in_circle(size);
                    // Add some spiky variation
                    leaf_pos = leaf_pos * (1.0f + rng.next_float(-0.3f, 0.5f));
                    break;
                case Shape::Scattered:
                    leaf_pos = rng.next_point_in_circle(size * 1.5f);
                    break;
            }
            
            leaf_positions.push_back(position + leaf_pos);
        }
    }
    
    // Calculate bounding box
    Rect2Df bounding_box() const noexcept {
        return Rect2Df{
            {position.x - size, position.y - size},
            {position.x + size, position.y + size}
        };
    }
};

// Complete tree structure
struct TreeStructure {
    std::shared_ptr<Branch> root;
    std::vector<std::shared_ptr<Branch>> all_branches;
    std::vector<LeafCluster> leaf_clusters;
    
    // Tree metadata
    TreeParameters parameters;
    Rect2Df bounding_box;
    uint32_t generation_id;
    
    // Constructor
    explicit TreeStructure(const TreeParameters& params) 
        : parameters(params), generation_id(0) {}
    
    // Calculate overall bounding box
    void calculate_bounding_box() {
        if (all_branches.empty()) {
            bounding_box = Rect2Df{{0, 0}, {0, 0}};
            return;
        }
        
        // Start with first branch
        bounding_box = all_branches[0]->bounding_box();
        
        // Expand for all branches
        for (const auto& branch : all_branches) {
            const auto branch_box = branch->bounding_box();
            bounding_box.min.x = std::min(bounding_box.min.x, branch_box.min.x);
            bounding_box.min.y = std::min(bounding_box.min.y, branch_box.min.y);
            bounding_box.max.x = std::max(bounding_box.max.x, branch_box.max.x);
            bounding_box.max.y = std::max(bounding_box.max.y, branch_box.max.y);
        }
        
        // Expand for leaf clusters
        for (const auto& cluster : leaf_clusters) {
            const auto cluster_box = cluster.bounding_box();
            bounding_box.min.x = std::min(bounding_box.min.x, cluster_box.min.x);
            bounding_box.min.y = std::min(bounding_box.min.y, cluster_box.min.y);
            bounding_box.max.x = std::max(bounding_box.max.x, cluster_box.max.x);
            bounding_box.max.y = std::max(bounding_box.max.y, cluster_box.max.y);
        }
    }
    
    // Get all leaf branches
    std::vector<std::shared_ptr<Branch>> get_leaf_branches() const {
        std::vector<std::shared_ptr<Branch>> leaves;
        if (root) {
            root->get_leaf_branches(leaves);
        }
        return leaves;
    }
    
    // Statistics
    size_t branch_count() const noexcept { return all_branches.size(); }
    size_t leaf_cluster_count() const noexcept { return leaf_clusters.size(); }
    int max_depth() const noexcept {
        int max_d = 0;
        for (const auto& branch : all_branches) {
            max_d = std::max(max_d, branch->depth_level);
        }
        return max_d;
    }
};

} // namespace pixeltree