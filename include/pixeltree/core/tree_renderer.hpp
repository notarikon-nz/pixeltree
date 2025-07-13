#pragma once
#include "pixel_buffer.hpp"
#include "tree_structure.hpp"
#include <cmath>

namespace pixeltree {

// High-performance tree renderer
class TreeRenderer {
public:
    // Render complete tree to pixel buffer
    PixelBuffer32 render(const TreeStructure& tree) const {
        const auto& params = tree.parameters;
        PixelBuffer32 buffer(params.canvas_width.get(), params.canvas_height.get());
        
        // Clear with transparent background
        buffer.clear(0x00000000);
        
        // Render branches first (back to front)
        render_branches(buffer, tree);
        
        // Render leaves on top
        render_leaves(buffer, tree);
        
        return buffer;
    }

private:
    // Render all branches using line drawing
    void render_branches(PixelBuffer32& buffer, const TreeStructure& tree) const {
        for (const auto& branch : tree.all_branches) {
            draw_thick_line(buffer, 
                           branch->start_point, 
                           branch->end_point,
                           branch->thickness,
                           branch->color.to_rgba());
        }
    }
    
    // Render all leaf clusters
    void render_leaves(PixelBuffer32& buffer, const TreeStructure& tree) const {
        for (const auto& cluster : tree.leaf_clusters) {
            draw_leaf_cluster(buffer, cluster);
        }
    }
    
    // Draw a thick line using Bresenham's algorithm with thickness
    void draw_thick_line(PixelBuffer32& buffer, 
                        Point2Df start, Point2Df end, 
                        float thickness, uint32_t color) const {
        // Convert to integer coordinates
        const int x0 = static_cast<int>(std::round(start.x));
        const int y0 = static_cast<int>(std::round(start.y));
        const int x1 = static_cast<int>(std::round(end.x));
        const int y1 = static_cast<int>(std::round(end.y));
        
        const int half_thickness = static_cast<int>(std::ceil(thickness * 0.5f));
        
        // Simple thick line by drawing multiple parallel lines
        for (int offset = -half_thickness; offset <= half_thickness; ++offset) {
            draw_line(buffer, x0 + offset, y0, x1 + offset, y1, color);
            draw_line(buffer, x0, y0 + offset, x1, y1 + offset, color);
        }
    }
    
    // Bresenham's line algorithm
    void draw_line(PixelBuffer32& buffer, 
                   int x0, int y0, int x1, int y1, uint32_t color) const {
        const int dx = std::abs(x1 - x0);
        const int dy = std::abs(y1 - y0);
        const int sx = x0 < x1 ? 1 : -1;
        const int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        
        int x = x0, y = y0;
        
        while (true) {
            if (buffer.contains(x, y)) {
                buffer(x, y) = color;
            }
            
            if (x == x1 && y == y1) break;
            
            const int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x += sx;
            }
            if (e2 < dx) {
                err += dx;
                y += sy;
            }
        }
    }
    
    // Draw a leaf cluster as filled circles
    void draw_leaf_cluster(PixelBuffer32& buffer, const LeafCluster& cluster) const {
        const int center_x = static_cast<int>(std::round(cluster.position.x));
        const int center_y = static_cast<int>(std::round(cluster.position.y));
        const int radius = static_cast<int>(std::ceil(cluster.size));
        
        const uint32_t color = cluster.color.to_rgba();
        
        // Draw filled circle using midpoint circle algorithm
        for (int y = -radius; y <= radius; ++y) {
            for (int x = -radius; x <= radius; ++x) {
                if (x * x + y * y <= radius * radius) {
                    const int pixel_x = center_x + x;
                    const int pixel_y = center_y + y;
                    
                    if (buffer.contains(pixel_x, pixel_y)) {
                        buffer(pixel_x, pixel_y) = color;
                    }
                }
            }
        }
    }
};

} // namespace pixeltree