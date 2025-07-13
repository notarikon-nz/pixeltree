#pragma once

#include "tree_structure.hpp"
#include "random.hpp"
#include <unordered_map>
#include <string>
#include <variant>
#include <functional>

namespace pixeltree {

// L-System rule types
struct GrowthRule {
    float length_factor = 1.0f;
    float thickness_factor = 1.0f;
    float angle_change = 0.0f;
};

struct SplitRule {
    int branch_count = 2;
    float angle_spread = 45.0f;
    float thickness_split = 0.7f;
};

struct TerminateRule {
    float probability = 1.0f;
};

using LSystemRule = std::variant<GrowthRule, SplitRule, TerminateRule>;

// L-System state for tree generation
struct LSystemState {
    Point2Df position;
    Point2Df direction;
    float thickness;
    int depth;
    Color color;
    
    LSystemState(Point2Df pos, Point2Df dir, float thick, int d, Color col)
        : position(pos), direction(dir), thickness(thick), depth(d), color(col) {}
};

// L-System based tree generator
class LSystemGenerator {
    std::unordered_map<char, LSystemRule> rules_;
    
public:
    // Set up default rules based on tree type
    void setup_rules(TreeType type) {
        rules_.clear();
        
        switch (type) {
            case TreeType::Oak:
                rules_['F'] = GrowthRule{1.0f, 0.9f, 0.0f};
                rules_['['] = SplitRule{2, 35.0f, 0.7f};
                rules_[']'] = TerminateRule{0.1f};
                break;
                
            case TreeType::Pine:
                rules_['F'] = GrowthRule{1.2f, 0.8f, 0.0f};
                rules_['['] = SplitRule{3, 25.0f, 0.6f};
                rules_[']'] = TerminateRule{0.2f};
                break;
                
            case TreeType::Palm:
                rules_['F'] = GrowthRule{1.5f, 0.9f, 10.0f};
                rules_['['] = SplitRule{5, 60.0f, 0.8f};
                rules_[']'] = TerminateRule{0.8f};
                break;
                
            default:
                // Default oak-like rules
                rules_['F'] = GrowthRule{1.0f, 0.9f, 0.0f};
                rules_['['] = SplitRule{2, 30.0f, 0.7f};
                rules_[']'] = TerminateRule{0.1f};
                break;
        }
    }
    
    // Generate L-System string
    std::string generate_string(const TreeParameters& params, Random& rng) const {
        std::string result = "F";
        
        for (int iteration = 0; iteration < params.branches.max_depth.get(); ++iteration) {
            std::string next_result;
            
            for (char c : result) {
                if (c == 'F') {
                    // Growth with potential branching
                    next_result += 'F';
                    if (rng.next_float() < params.branches.branch_probability.get()) {
                        next_result += "[+F][-F]";
                    }
                } else {
                    next_result += c;
                }
            }
            
            result = next_result;
        }
        
        return result;
    }
    
    // Convert L-System string to tree structure
    std::unique_ptr<TreeStructure> string_to_tree(const std::string& lstring,
                                                  const TreeParameters& params,
                                                  Random& rng) const {
        auto tree = std::make_unique<TreeStructure>(params);
        
        // Starting state
        const Point2Df start_pos{params.canvas_width.get() * 0.5f, 
                                params.canvas_height.get() * 0.9f};
        const Point2Df start_dir{0.0f, -1.0f};
        
        std::vector<LSystemState> state_stack;
        LSystemState current_state{start_pos, start_dir, 
                                  params.branches.base_thickness.get(), 
                                  0, params.trunk.base_color};
        
        std::shared_ptr<Branch> current_branch = nullptr;
        
        for (char c : lstring) {
            switch (c) {
                case 'F': {
                    // Forward movement - create branch
                    const float branch_length = 15.0f * params.overall_scale.get();
                    const Point2Df end_pos = current_state.position + 
                                           current_state.direction * branch_length;
                    
                    auto branch = std::make_shared<Branch>(current_state.position, 
                                                         end_pos, 
                                                         current_state.thickness,
                                                         current_state.depth);
                    branch->color = current_state.color;
                    
                    if (!tree->root) {
                        tree->root = branch;
                    } else if (current_branch) {
                        current_branch->children.push_back(branch);
                        branch->parent = current_branch;
                    }
                    
                    tree->all_branches.push_back(branch);
                    current_branch = branch;
                    current_state.position = end_pos;
                    
                    break;
                }
                
                case '[': {
                    // Push state and start branching
                    state_stack.push_back(current_state);
                    break;
                }
                
                case ']': {
                    // Pop state
                    if (!state_stack.empty()) {
                        current_state = state_stack.back();
                        state_stack.pop_back();
                    }
                    break;
                }
                
                case '+': {
                    // Turn right
                    const float angle = rng.next_float(-45.0f, 45.0f) * 
                                      params.branches.branch_angle_variation.get();
                    current_state.direction = rotate_vector(current_state.direction, angle);
                    break;
                }
                
                case '-': {
                    // Turn left
                    const float angle = rng.next_float(-45.0f, 45.0f) * 
                                      params.branches.branch_angle_variation.get();
                    current_state.direction = rotate_vector(current_state.direction, -angle);
                    break;
                }
            }
            
            // Apply thickness decay
            current_state.thickness *= params.branches.thickness_decay.get();
            current_state.depth++;
        }
        
        return tree;
    }

private:
    // Rotate a 2D vector by angle in degrees
    Point2Df rotate_vector(const Point2Df& vec, float angle_degrees) const {
        const float angle_rad = angle_degrees * M_PI / 180.0f;
        const float cos_a = std::cos(angle_rad);
        const float sin_a = std::sin(angle_rad);
        
        return Point2Df{
            vec.x * cos_a - vec.y * sin_a,
            vec.x * sin_a + vec.y * cos_a
        };
    }
};

} // namespace pixeltree