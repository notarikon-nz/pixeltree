#pragma once

// Version information
#include "config.hpp"

// Core functionality
#include "core/math_types.hpp"
#include "core/tree_parameters.hpp"
#include "core/tree_structure.hpp"
#include "core/pixel_buffer.hpp"
#include "core/tree_generator.hpp"
#include "core/random.hpp"

// Export functionality
namespace pixeltree {

// Version information
constexpr int version_major() noexcept { return PIXELTREE_VERSION_MAJOR; }
constexpr int version_minor() noexcept { return PIXELTREE_VERSION_MINOR; }
constexpr int version_patch() noexcept { return PIXELTREE_VERSION_PATCH; }
constexpr const char* version_string() noexcept { return PIXELTREE_VERSION_STRING; }

// Feature detection
constexpr bool has_png_support() noexcept {
#ifdef PIXELTREE_HAS_PNG
    return true;
#else
    return false;
#endif
}

constexpr bool has_openmp_support() noexcept {
#ifdef PIXELTREE_HAS_OPENMP
    return true;
#else
    return false;
#endif
}

constexpr bool has_simd_support() noexcept {
#if defined(PIXELTREE_HAS_SSE2) || defined(PIXELTREE_HAS_AVX2)
    return true;
#else
    return false;
#endif
}

// Quick generation functions for ease of use
inline auto generate_oak_tree(int width = 128, int height = 128, uint32_t seed = 0) {
    TreeGenerator32 generator(seed);
    auto params = TreePresets::oak();
    params.canvas_width = width;
    params.canvas_height = height;
    return generator.generate(params);
}

inline auto generate_pine_tree(int width = 128, int height = 128, uint32_t seed = 0) {
    TreeGenerator32 generator(seed);
    auto params = TreePresets::pine();
    params.canvas_width = width;
    params.canvas_height = height;
    return generator.generate(params);
}

inline auto generate_palm_tree(int width = 128, int height = 128, uint32_t seed = 0) {
    TreeGenerator32 generator(seed);
    auto params = TreePresets::palm();
    params.canvas_width = width;
    params.canvas_height = height;
    return generator.generate(params);
}

} // namespace pixeltree