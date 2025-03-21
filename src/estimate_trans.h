#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include <random>
#include "vector.h"
#include "comb.h"
#include "power_series.h"
#include "pcg.h"

float transEstimator(Vec3 start_pos, Vec3 end_pos, 
                     float (*getDensity)(const Vec3),
                     UniformRandom& float_rng) {
    int M = 12;
    float K = 2;
    float c = 2.5;
    std::vector<float> X;
    std::vector<float> Q;
    for (int i = 0; i < K + 1; i++) {
        X.push_back(combEstimator(start_pos, end_pos, M, getDensity, float_rng));
        Q.push_back(1);
    }
    float q_i = 1;
    int i = 1;
    while (true) {
        float prob = c / (K + i);
        if (float_rng.next_float() > prob) {
            break;
        }
        q_i *= prob;
        X.push_back(combEstimator(start_pos, end_pos, M, getDensity, float_rng));
        Q.push_back(q_i);
        i++;
    }
    return compute_T(X, Q);
}