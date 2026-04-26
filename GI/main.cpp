#include "app.hpp"

// std
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>
#include <stdexcept>

int main() {
    Application app{};

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    std::vector<std::string> filenames = {
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        "shaders/point_light.frag.spv",
        "shaders/point_light.vert.spv"
    };

	for (const auto& filename : filenames) {
        if (std::filesystem::remove(filename)) {
            std::cout << "File deleted: " << filename << "\n";
        }
        else {
            std::cout << "No file with name: " << filename << "\n";
        }
    }
    return EXIT_SUCCESS;
}