#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono> 
#include "vector.h"
#include "FastNoiseLite.h"
#include "save_exr.h"
#include "estimate_trans.h"
#include "pcg.h"

FastNoiseLite noiseGen;

void initNoise() {
    noiseGen.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noiseGen.SetFractalType(FastNoiseLite::FractalType_FBm);
    noiseGen.SetFractalOctaves(5);
    noiseGen.SetFrequency(0.5f);
}

float density(const Vec3 p) {
    float baseNoise = noiseGen.GetNoise(p.x, p.y, p.z);
    float d = std::max(0.0f, std::min((baseNoise + 1.0f) * 0.5f, 1.0f));

    float dist = p.length();
    float sphereFalloff = std::max(0.0f, std::min(1.0f - dist / 2.0f, 1.0f));

    return d * sphereFalloff;
}

float shadow(const Vec3& point, const Vec3& lightDir, UniformRandom& float_rng) {
    float t = 0.0f;
    float maxDist = 3.0f;
    float stepSize = 0.02f;
    float transmittance = 1.0f;

    float radious = 2.0f;
    for (int i = 0; i < 100 && t < maxDist && transmittance > 0.01f; i++) {
        Vec3 start_pos = point + lightDir * t;
        if (start_pos.length() > radious) {
            t += stepSize;
            continue;
        }
        t += stepSize;
        Vec3 end_pos = point + lightDir * t;

        transmittance = transmittance 
                        * transEstimator(start_pos, end_pos, density,
                                         float_rng);
    }
    return transmittance;
}

float hgPhase(float cosTheta, float g) {
    float denom = 1.0f + g * g - 2.0f * g * cosTheta;
    return (1.0f - g * g) / (4.0f * M_PI * denom * std::sqrt(denom));
}

Vec3 emission(const Vec3& p, const Vec3& rayDir, UniformRandom& float_rng) {
    float g = 0.2f;
    float sigma_s = 1.0f;
    Vec3 sunColor = Vec3(20.0f, 8.0f, 7.0f) * 3.5;
    Vec3 lightDir = Vec3(.0f, .0f, -1.0f).normalized();
    float cosTheta = dot(rayDir, lightDir);
    float phase = hgPhase(cosTheta, g);
    return shadow(p, lightDir, float_rng) * sigma_s * phase * sunColor;
}

Vec4 raymarch(const Vec3& rayOrigin, const Vec3& rayDir,
              float tMin, float tMax, float stepSize,
              UniformRandom& float_rng) {
    float t = tMin;
    const int maxSteps = 512;
    float trans_low_limit = 0.001;

    float transmittance = 1.0f;
    Vec3 accumulatedColor(0.0f, 0.0f, 0.0f);
    int steps = 0;

    float radious = 2.0f;

    while (t < tMax && transmittance > trans_low_limit && steps < maxSteps) {
        Vec3 start_pos = rayOrigin + rayDir * t;
        if (start_pos.length() > radious) {
            t += stepSize;
            steps++;
            continue;
        }

        t += stepSize;
        Vec3 end_pos = rayOrigin + rayDir * t;

        float estExp = transEstimator(start_pos, end_pos, density, float_rng);
        transmittance = transmittance * estExp;

        accumulatedColor = accumulatedColor
                           + transmittance * (1 - estExp)
                           * emission(end_pos, rayDir, float_rng);

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

    UniformRandom float_rng(42, 0.0f, 1.0f);
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

            Vec4 rawColor = raymarch(cameraPos, rayDir, tMin, tMax, stepSize, float_rng);
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
