#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono> 
#include "vector.h"
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfRgba.h>
#include <OpenEXR/ImfArray.h>
#include "FastNoiseLite.h"

FastNoiseLite noiseGen;

void initNoise() {
    noiseGen.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noiseGen.SetFractalType(FastNoiseLite::FractalType_FBm);
    noiseGen.SetFractalOctaves(5);
    noiseGen.SetFrequency(0.5f);
}

float density(const Vec3& p) {
    float baseNoise = noiseGen.GetNoise(p.x, p.y, p.z);
    float d = std::max(0.0f, std::min((baseNoise + 1.0f) * 0.5f, 1.0f));

    float dist = p.length();
    float sphereFalloff = std::max(0.0f, std::min(1.0f - dist / 2.0f, 1.0f));

    return d * sphereFalloff;
}

float shadow(const Vec3& point, const Vec3& lightDir) {
    float t = 0.0f;
    float maxDist = 3.0f;
    float stepSize = 0.1f;
    float transmittance = 1.0f;
    float tau = 0;

    for (int i = 0; i < 100 && t < maxDist && transmittance > 0.01f; i++) {
        Vec3 samplePos = point + lightDir * t;
        float dens = density(samplePos);
        tau += dens * stepSize;
        transmittance = std::exp(-tau);
        t += stepSize;
    }
    return transmittance;
}

float hgPhase(float cosTheta, float g) {
    float denom = 1.0f + g * g - 2.0f * g * cosTheta;
    return (1.0f - g * g) / (4.0f * M_PI * denom * std::sqrt(denom));
}

Vec3 emission(const Vec3& p, const Vec3& rayDir) {
    float g = 0.2f;
    float sigma_s = 1.0f;
    Vec3 sunColor = Vec3(20.0f, 8.0f, 7.0f) * 3.5;
    Vec3 lightDir = Vec3(.0f, .0f, -1.0f).normalized();
    float cosTheta = dot(rayDir, lightDir);
    float phase = hgPhase(cosTheta, g);
    return shadow(p, lightDir) * sigma_s * phase * sunColor;
}

Vec4 raymarch(const Vec3& rayOrigin, const Vec3& rayDir, 
              float tMin, float tMax, float stepSize) {
    float t = tMin + stepSize / 2;
    const int maxSteps = 512;
    float trans_low_limit = 0.001;

    float transmittance = 1.0f;
    Vec3 accumulatedColor(0.0f, 0.0f, 0.0f);
    int steps = 0;
    float tau = 0.0f;

    while (t < tMax && transmittance > trans_low_limit && steps < maxSteps) {
        Vec3 pos = rayOrigin + rayDir * t;
        float dens = density(pos);

        tau += dens * stepSize;
        transmittance = std::exp(-tau);

        Vec3 radiance = emission(pos, rayDir);
        accumulatedColor = accumulatedColor 
                           + transmittance * (1 - std::exp(-dens * stepSize)) * radiance;

        t += stepSize;
        steps++;
    }

    return Vec4(accumulatedColor.x, accumulatedColor.y, accumulatedColor.z, 1.0f - transmittance);
}

int main() {
    const int width = 128;
    const int height = 128;
    // const int width = 512;
    // const int height = 512;

    initNoise();

    Vec3 backgroundColor(0.5f, 0.7f, 1.0f);
    Vec3 cameraPos(0.0f, 0.0f, -3.0f);
    std::vector<Vec4> pixels(width * height);

    int progressBarWidth = 50;

    auto startTime = std::chrono::high_resolution_clock::now();
    float aspect = width / (float)height;

    float tMin = 0.0f;
    float tMax = 5.0f;
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
            float u = ((i / (float)width) * 2.0f - 1.0f) * aspect;
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
