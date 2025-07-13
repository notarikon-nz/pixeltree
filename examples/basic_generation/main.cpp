#include <pixeltree/pixeltree.hpp>
#include <iostream>
#include <fstream>

using namespace pixeltree;

// Simple PPM image writer for demonstration
void write_ppm(const std::string& filename, const PixelBuffer32& buffer) {
    std::ofstream file(filename);
    file << "P3\n" << buffer.width() << " " << buffer.height() << "\n255\n";
    
    for (size_t y = 0; y < buffer.height(); ++y) {
        for (size_t x = 0; x < buffer.width(); ++x) {
            const uint32_t pixel = buffer(x, y);
            const uint8_t r = (pixel >> 24) & 0xFF;
            const uint8_t g = (pixel >> 16) & 0xFF;
            const uint8_t b = (pixel >> 8) & 0xFF;
            file << static_cast<int>(r) << " " << static_cast<int>(g) << " " << static_cast<int>(b) << " ";
        }
        file << "\n";
    }
}

int main() {
    std::cout << "PixelTree Basic Generation Example\n";
    std::cout << "====================================\n";
    
    // Create generator
    TreeGenerator32 generator;
    
    // Generate different tree types
    const std::vector<std::pair<std::string, TreeParameters>> trees = {
        {"oak", TreePresets::oak()},
        {"pine", TreePresets::pine()},
        {"palm", TreePresets::palm()},
        {"dead", TreePresets::dead()}
    };
    
    for (const auto& [name, params] : trees) {
        std::cout << "Generating " << name << " tree... ";
        
        auto [buffer, metadata] = generator.generate(params);
        
        std::cout << "Done!\n";
        std::cout << "  Branches: " << metadata.branch_count << "\n";
        std::cout << "  Leaves: " << metadata.leaf_count << "\n";
        std::cout << "  Generation time: " << metadata.generation_time_ms << "ms\n";
        
        // Save as PPM image
        write_ppm(name + "_tree.ppm", buffer);
        std::cout << "  Saved as " << name << "_tree.ppm\n\n";
    }
    
    return 0;
}