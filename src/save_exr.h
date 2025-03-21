#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfRgba.h>
#include <OpenEXR/ImfArray.h>
#include "vector.h"

void saveEXR(const std::vector<Vec4>& pixels, int width, int height, const char* filename) {
    Imf::Array2D<Imf::Rgba> exrPixels;
    exrPixels.resizeErase(height, width);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const Vec4& color = pixels[y * width + x];
            exrPixels[y][x] = Imf::Rgba(color.x, color.y, color.z, color.w);
        }
    }

    try {
        Imf::RgbaOutputFile file(filename, width, height, Imf::WRITE_RGBA);
        file.setFrameBuffer(&exrPixels[0][0], 1, width);
        file.writePixels(height);
        std::cout << "Saved EXR file: " << filename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to save EXR file: " << e.what() << std::endl;
    }
}
