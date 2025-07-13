#pragma once
#include "math_types.hpp"
#include <memory>
#include <vector>
#include <algorithm>

namespace pixeltree {

// Generic pixel buffer with RAII management
template<typename PixelType>
class PixelBuffer {
    std::unique_ptr<PixelType[]> data_;
    size_t width_, height_;
    
public:
    // Constructors
    PixelBuffer() : width_(0), height_(0) {}
    
    PixelBuffer(size_t width, size_t height) 
        : data_(std::make_unique<PixelType[]>(width * height))
        , width_(width), height_(height) {
        clear();
    }
    
    // Move semantics
    PixelBuffer(PixelBuffer&& other) noexcept 
        : data_(std::move(other.data_))
        , width_(other.width_), height_(other.height_) {
        other.width_ = other.height_ = 0;
    }
    
    PixelBuffer& operator=(PixelBuffer&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
            width_ = other.width_;
            height_ = other.height_;
            other.width_ = other.height_ = 0;
        }
        return *this;
    }
    
    // No copy (expensive operation should be explicit)
    PixelBuffer(const PixelBuffer&) = delete;
    PixelBuffer& operator=(const PixelBuffer&) = delete;
    
    // Deep copy method
    PixelBuffer clone() const {
        PixelBuffer result(width_, height_);
        std::copy(begin(), end(), result.begin());
        return result;
    }
    
    // Accessors
    size_t width() const noexcept { return width_; }
    size_t height() const noexcept { return height_; }
    size_t size() const noexcept { return width_ * height_; }
    bool empty() const noexcept { return size() == 0; }
    
    // Data access
    PixelType* data() noexcept { return data_.get(); }
    const PixelType* data() const noexcept { return data_.get(); }
    
    // Iterator support
    PixelType* begin() noexcept { return data_.get(); }
    PixelType* end() noexcept { return data_.get() + size(); }
    const PixelType* begin() const noexcept { return data_.get(); }
    const PixelType* end() const noexcept { return data_.get() + size(); }
    
    // Pixel access with bounds checking
    PixelType& at(size_t x, size_t y) {
        if (x >= width_ || y >= height_) {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }
        return data_[y * width_ + x];
    }
    
    const PixelType& at(size_t x, size_t y) const {
        if (x >= width_ || y >= height_) {
            throw std::out_of_range("Pixel coordinates out of bounds");
        }
        return data_[y * width_ + x];
    }
    
    // Unchecked pixel access (for performance)
    PixelType& operator()(size_t x, size_t y) noexcept {
        return data_[y * width_ + x];
    }
    
    const PixelType& operator()(size_t x, size_t y) const noexcept {
        return data_[y * width_ + x];
    }
    
    // Utility methods
    void clear(PixelType value = PixelType{}) {
        std::fill(begin(), end(), value);
    }
    
    void resize(size_t new_width, size_t new_height) {
        if (new_width != width_ || new_height != height_) {
            data_ = std::make_unique<PixelType[]>(new_width * new_height);
            width_ = new_width;
            height_ = new_height;
            clear();
        }
    }
    
    // Check if coordinates are within bounds
    bool contains(int x, int y) const noexcept {
        return x >= 0 && x < static_cast<int>(width_) && 
               y >= 0 && y < static_cast<int>(height_);
    }
    
    // Blitting operations
    void blit(const PixelBuffer& source, Point2Di position) {
        const int src_width = static_cast<int>(source.width());
        const int src_height = static_cast<int>(source.height());
        
        for (int y = 0; y < src_height; ++y) {
            for (int x = 0; x < src_width; ++x) {
                const int dest_x = position.x + x;
                const int dest_y = position.y + y;
                
                if (contains(dest_x, dest_y)) {
                    (*this)(dest_x, dest_y) = source(x, y);
                }
            }
        }
    }
    
    // Alpha blending for RGBA pixels
    void blit_with_alpha(const PixelBuffer& source, Point2Di position) 
        requires (std::is_same_v<PixelType, uint32_t>) {
        const int src_width = static_cast<int>(source.width());
        const int src_height = static_cast<int>(source.height());
        
        for (int y = 0; y < src_height; ++y) {
            for (int x = 0; x < src_width; ++x) {
                const int dest_x = position.x + x;
                const int dest_y = position.y + y;
                
                if (contains(dest_x, dest_y)) {
                    const uint32_t src_pixel = source(x, y);
                    const uint32_t dest_pixel = (*this)(dest_x, dest_y);
                    (*this)(dest_x, dest_y) = blend_pixels(dest_pixel, src_pixel);
                }
            }
        }
    }

private:
    // Alpha blending for RGBA pixels (format: RGBA)
    uint32_t blend_pixels(uint32_t background, uint32_t foreground) const {
        const uint8_t alpha = foreground & 0xFF;
        if (alpha == 0) return background;
        if (alpha == 255) return foreground;
        
        const float a = alpha / 255.0f;
        const float inv_a = 1.0f - a;
        
        const uint8_t bg_r = (background >> 24) & 0xFF;
        const uint8_t bg_g = (background >> 16) & 0xFF;
        const uint8_t bg_b = (background >> 8) & 0xFF;
        
        const uint8_t fg_r = (foreground >> 24) & 0xFF;
        const uint8_t fg_g = (foreground >> 16) & 0xFF;
        const uint8_t fg_b = (foreground >> 8) & 0xFF;
        
        const uint8_t result_r = static_cast<uint8_t>(bg_r * inv_a + fg_r * a);
        const uint8_t result_g = static_cast<uint8_t>(bg_g * inv_a + fg_g * a);
        const uint8_t result_b = static_cast<uint8_t>(bg_b * inv_a + fg_b * a);
        
        return (result_r << 24) | (result_g << 16) | (result_b << 8) | 255;
    }
};

// Common pixel buffer types
using PixelBuffer32 = PixelBuffer<uint32_t>;
using PixelBuffer8 = PixelBuffer<uint8_t>;

} // namespace pixeltree