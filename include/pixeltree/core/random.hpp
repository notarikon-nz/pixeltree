#pragma once
#include <random>
#include <type_traits>
#include "math_types.hpp"

namespace pixeltree {

// Thread-safe random number generator
class Random {
    mutable std::mt19937 generator_;
    
public:
    explicit Random(uint32_t seed = std::random_device{}()) 
        : generator_(seed) {}
    
    // Generate random float in range [0, 1)
    float next_float() const {
        static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        return dist(generator_);
    }
    
    // Generate random float in range [min, max)
    float next_float(float min, float max) const {
        return min + next_float() * (max - min);
    }
    
    // Generate random int in range [min, max]
    int next_int(int min, int max) const {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator_);
    }
    
    // Generate random boolean with given probability
    bool next_bool(float probability = 0.5f) const {
        return next_float() < probability;
    }
    
    // Generate random point in circle
    Point2Df next_point_in_circle(float radius = 1.0f) const {
        const float angle = next_float(0.0f, 2.0f * M_PI);
        const float r = std::sqrt(next_float()) * radius;
        return {r * std::cos(angle), r * std::sin(angle)};
    }
    
    // Generate random point in rectangle
    Point2Df next_point_in_rect(const Rect2Df& rect) const {
        return {
            next_float(rect.min.x, rect.max.x),
            next_float(rect.min.y, rect.max.y)
        };
    }
};

} // namespace pixeltree