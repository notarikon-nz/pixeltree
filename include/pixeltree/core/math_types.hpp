#pragma once

#include <stdint.h>
#include <cmath>
#include <array>
#include <type_traits>

namespace pixeltree {

// 2D Point with template precision
template<typename T>
struct Point2D {
    T x, y;
    
    constexpr Point2D() noexcept : x(0), y(0) {}
    constexpr Point2D(T x_, T y_) noexcept : x(x_), y(y_) {}
    
    // Vector operations
    constexpr Point2D operator+(const Point2D& other) const noexcept {
        return {x + other.x, y + other.y};
    }
    
    constexpr Point2D operator-(const Point2D& other) const noexcept {
        return {x - other.x, y - other.y};
    }
    
    constexpr Point2D operator*(T scalar) const noexcept {
        return {x * scalar, y * scalar};
    }
    
    constexpr T dot(const Point2D& other) const noexcept {
        return x * other.x + y * other.y;
    }
    
    T length() const noexcept {
        return std::sqrt(x * x + y * y);
    }
    
    Point2D normalized() const noexcept {
        const T len = length();
        return len > 0 ? Point2D{x / len, y / len} : Point2D{};
    }
};

using Point2Df = Point2D<float>;
using Point2Di = Point2D<int>;

// Bounding rectangle
template<typename T>
struct Rect2D {
    Point2D<T> min, max;
    
    constexpr Rect2D() noexcept = default;
    constexpr Rect2D(Point2D<T> min_, Point2D<T> max_) noexcept : min(min_), max(max_) {}
    
    constexpr T width() const noexcept { return max.x - min.x; }
    constexpr T height() const noexcept { return max.y - min.y; }
    constexpr Point2D<T> center() const noexcept { return (min + max) * T(0.5); }
    
    constexpr bool contains(const Point2D<T>& point) const noexcept {
        return point.x >= min.x && point.x <= max.x && 
               point.y >= min.y && point.y <= max.y;
    }
};

using Rect2Df = Rect2D<float>;
using Rect2Di = Rect2D<int>;

// Color with alpha
struct Color {
    uint8_t r, g, b, a;
    
    constexpr Color() noexcept : r(0), g(0), b(0), a(255) {}
    constexpr Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) noexcept 
        : r(r_), g(g_), b(b_), a(a_) {}
    
    // Convert to 32-bit RGBA
    constexpr uint32_t to_rgba() const noexcept {
        return (uint32_t(r) << 24) | (uint32_t(g) << 16) | (uint32_t(b) << 8) | uint32_t(a);
    }
    
    // Create from 32-bit RGBA
    static constexpr Color from_rgba(uint32_t rgba) noexcept {
        return Color{
            uint8_t((rgba >> 24) & 0xFF),
            uint8_t((rgba >> 16) & 0xFF),
            uint8_t((rgba >> 8) & 0xFF),
            uint8_t(rgba & 0xFF)
        };
    }
    
    // Linear interpolation
    Color lerp(const Color& other, float t) const noexcept {
        const float inv_t = 1.0f - t;
        return Color{
            uint8_t(r * inv_t + other.r * t),
            uint8_t(g * inv_t + other.g * t),
            uint8_t(b * inv_t + other.b * t),
            uint8_t(a * inv_t + other.a * t)
        };
    }
};

// Constrained numeric types for parameter validation
template<typename T, T Min, T Max>
class BoundedValue {
    
    T value_;
    
public:
    constexpr BoundedValue(T value = Min) noexcept : value_(std::clamp(value, Min, Max)) {}
    
    constexpr operator T() const noexcept { return value_; }
    constexpr T get() const noexcept { return value_; }
    
    constexpr BoundedValue& operator=(T value) noexcept {
        value_ = std::clamp(value, Min, Max);
        return *this;
    }
    
    static constexpr T min_value() noexcept { return Min; }
    static constexpr T max_value() noexcept { return Max; }
};

// Common bounded types
using BoundedFloat01 = BoundedValue<float, 0.0f, 1.0f>;
using BoundedFloat10 = BoundedValue<float, 0.1f, 10.0f>;
using BoundedAngle = BoundedValue<float, 0.0f, 360.0f>;

} // namespace pixeltree