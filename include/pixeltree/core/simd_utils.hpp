#pragma once
#include "config.hpp"
#include <cstdint>
#include <cstring>

namespace pixeltree::simd {

// SIMD-optimized pixel operations
class PixelOperations {
public:
    // Clear buffer with SIMD acceleration
    static void clear_buffer(uint32_t* data, size_t count, uint32_t value) {
#ifdef PIXELTREE_HAS_AVX2
        clear_buffer_avx2(data, count, value);
#elif defined(PIXELTREE_HAS_SSE2)
        clear_buffer_sse2(data, count, value);
#else
        clear_buffer_scalar(data, count, value);
#endif
    }
    
    // Alpha blend arrays of pixels
    static void alpha_blend(uint32_t* dest, const uint32_t* src, size_t count) {
#ifdef PIXELTREE_HAS_AVX2
        alpha_blend_avx2(dest, src, count);
#elif defined(PIXELTREE_HAS_SSE2)
        alpha_blend_sse2(dest, src, count);
#else
        alpha_blend_scalar(dest, src, count);
#endif
    }

private:
    // Scalar fallback implementations
    static void clear_buffer_scalar(uint32_t* data, size_t count, uint32_t value) {
        for (size_t i = 0; i < count; ++i) {
            data[i] = value;
        }
    }
    
    static void alpha_blend_scalar(uint32_t* dest, const uint32_t* src, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            dest[i] = blend_pixel_scalar(dest[i], src[i]);
        }
    }
    
    static uint32_t blend_pixel_scalar(uint32_t bg, uint32_t fg) {
        const uint8_t alpha = fg & 0xFF;
        if (alpha == 0) return bg;
        if (alpha == 255) return fg;
        
        const float a = alpha / 255.0f;
        const float inv_a = 1.0f - a;
        
        const uint8_t bg_r = (bg >> 24) & 0xFF;
        const uint8_t bg_g = (bg >> 16) & 0xFF;
        const uint8_t bg_b = (bg >> 8) & 0xFF;
        
        const uint8_t fg_r = (fg >> 24) & 0xFF;
        const uint8_t fg_g = (fg >> 16) & 0xFF;
        const uint8_t fg_b = (fg >> 8) & 0xFF;
        
        const uint8_t r = static_cast<uint8_t>(bg_r * inv_a + fg_r * a);
        const uint8_t g = static_cast<uint8_t>(bg_g * inv_a + fg_g * a);
        const uint8_t b = static_cast<uint8_t>(bg_b * inv_a + fg_b * a);
        
        return (r << 24) | (g << 16) | (b << 8) | 255;
    }

#ifdef PIXELTREE_HAS_SSE2
    static void clear_buffer_sse2(uint32_t* data, size_t count, uint32_t value) {
        const __m128i fill_value = _mm_set1_epi32(value);
        const size_t simd_count = count / 4;
        const size_t remainder = count % 4;
        
        for (size_t i = 0; i < simd_count; ++i) {
            _mm_storeu_si128(reinterpret_cast<__m128i*>(data + i * 4), fill_value);
        }
        
        // Handle remainder
        for (size_t i = simd_count * 4; i < count; ++i) {
            data[i] = value;
        }
    }
    
    static void alpha_blend_sse2(uint32_t* dest, const uint32_t* src, size_t count) {
        // Simplified SSE2 alpha blending
        const size_t simd_count = count / 4;
        const size_t remainder = count % 4;
        
        for (size_t i = 0; i < simd_count; ++i) {
            // Process 4 pixels at once (simplified implementation)
            for (int j = 0; j < 4; ++j) {
                dest[i * 4 + j] = blend_pixel_scalar(dest[i * 4 + j], src[i * 4 + j]);
            }
        }
        
        // Handle remainder
        for (size_t i = simd_count * 4; i < count; ++i) {
            dest[i] = blend_pixel_scalar(dest[i], src[i]);
        }
    }
#endif

#ifdef PIXELTREE_HAS_AVX2
    static void clear_buffer_avx2(uint32_t* data, size_t count, uint32_t value) {
        const __m256i fill_value = _mm256_set1_epi32(value);
        const size_t simd_count = count / 8;
        const size_t remainder = count % 8;
        
        for (size_t i = 0; i < simd_count; ++i) {
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(data + i * 8), fill_value);
        }
        
        // Handle remainder
        for (size_t i = simd_count * 8; i < count; ++i) {
            data[i] = value;
        }
    }
    
    static void alpha_blend_avx2(uint32_t* dest, const uint32_t* src, size_t count) {
        // Simplified AVX2 alpha blending
        const size_t simd_count = count / 8;
        const size_t remainder = count % 8;
        
        for (size_t i = 0; i < simd_count; ++i) {
            // Process 8 pixels at once (simplified implementation)
            for (int j = 0; j < 8; ++j) {
                dest[i * 8 + j] = blend_pixel_scalar(dest[i * 8 + j], src[i * 8 + j]);
            }
        }
        
        // Handle remainder
        for (size_t i = simd_count * 8; i < count; ++i) {
            dest[i] = blend_pixel_scalar(dest[i], src[i]);
        }
    }
#endif
};

} // namespace pixeltree::simd