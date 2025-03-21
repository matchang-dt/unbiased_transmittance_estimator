#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>
#include "vector.h"
#include "save_exr.h"

float density(const Vec3& p) {
    float radius = 2.0f;
    float dist = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (dist > radius) return 0.0f;

    return (1.0f - dist / radius);
}

Vec3 emission(const Vec3& p) {
    float radius = 2.0f;
    float dist = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    if (dist > radius) return Vec3(0.0f, 0.0f, 0.0f);
    return Vec3(1.0f, 0.5f, 0.35f);
}

Vec4 raymarch(const Vec3& rayOrigin, const Vec3& rayDir,
              float tMin, float tMax, float stepSize) {
    float t = tMin + stepSize / 2;

    float transmittance = 1.0f;
    Vec3 accumulatedColor(0.0f, 0.0f, 0.0f);

    const float sigma_s = 1.0f;
    const float sigma_a = 1.0f;

    const int maxSteps = 512;
    int steps = 0;
    float tau = 0.0;

    while (t < tMax && transmittance > 0.01f && steps < maxSteps) {
        Vec3 pos = rayOrigin + rayDir * t;

        float dens = density(pos);
        tau += dens * stepSize;
        transmittance = std::exp(-tau);

        Vec3 radiance = emission(pos);
        accumulatedColor = accumulatedColor 
                           + transmittance * (1 - std::exp(-dens * stepSize)) * radiance;

        t += stepSize;
        steps++;
    }

    return Vec4(accumulatedColor.x, accumulatedColor.y, accumulatedColor.z, 1.0f - transmittance);
}

int main() {
    const int width = 400;
    const int height = 400;

    Vec3 backgroundColor(0.5f, 0.7f, 1.0f);
    Vec3 cameraPos(0.0f, 0.0f, -3.0f);
    std::vector<Vec4> pixels(width * height);

    int progressBarWidth = 50;

    auto startTime = std::chrono::high_resolution_clock::now();
    float aspect = width / (float)height;

    float tMin = 0.0f;
    float tMax = 10.0f;
    float stepSize = 0.02f;

    for (int j = 0; j < height; ++j) {
        auto nowTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(nowTime - startTime).count();

        float progress = (float)(j + 1) / height;
        int pos = progress * progressBarWidth;

        std::cout << "\r[";
        for (int i = 0; i < progressBarWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << "% (" << j + 1 << "/" << height << ") "
        << "Elapsed: " << elapsed << "s   " << std::flush;

        for (int i = 0; i < width; ++i) {
            float u = (i / (float)width) * 2.0f - 1.0f;
            float v = (j / (float)height) * 2.0f - 1.0f;

            Vec3 rayDir(u, v, 1.0f);
            rayDir = rayDir.normalized();

            Vec4 rawColor = raymarch(cameraPos, rayDir, tMin, tMax, stepSize);
            float alpha = rawColor.w;
            Vec3 finalColor = Vec3(rawColor.x, rawColor.y, rawColor.z) * alpha +
                              backgroundColor * (1.0f - alpha);
            pixels[j * width + i] = Vec4(finalColor.x, finalColor.y, finalColor.z, 1.0f);
        }
    }
    std::cout << std::endl;
    saveEXR(pixels, width, height, "output.exr");
    return 0;
}
