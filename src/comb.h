#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include "vector.h"
#include "pcg.h"

float evaluateDensity(Vec3 p) {
    return 0.5f * std::exp(-p.length());
}

float combEstimator(Vec3 start_pos, Vec3 end_pos, 
                    int M, float (*getDensity)(Vec3),
                    UniformRandom& float_rng) {
    float L = (end_pos - start_pos).length();
    Vec3 rayDir = (end_pos - start_pos).normalized();

    float r = float_rng.next_float();
    
    float step = L / M;
    float tau = 0.0f;
    
    for (int j = 0; j < M; j++) {
        float t_j = std::fmod(r + j * step, L);

        tau += getDensity(start_pos + t_j * rayDir) * step;
    }
    
    float X = -tau;
    return X;
}