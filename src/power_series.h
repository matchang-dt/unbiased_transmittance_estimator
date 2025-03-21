#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>

float f_N(float p, const std::vector<float>& Y, const std::vector<float>& Q) {
    int N = Y.size();

    std::vector<float> shiftedY(N);
    for (int i = 0; i < N; i++) {
        shiftedY[i] = Y[i] - p;
    }

    std::vector<float> P(N);
    for (int i = 0; i < N; i++) {
        float sum = 0;
        for (int j = 0; j < N; j++) {
            sum += std::pow(shiftedY[j], (i + 1));
        }
        P[i] = sum;
    }

    std::vector<float> S(N);
    for (int i = 0; i < N; i++) {
        float sum = 0;
        int coef = 1;
        for (int j = 0; j < i; j++) {
            sum += coef * S[i-1-j] * P[j];
            coef *= -1;
        }
        sum += coef * P[i];
        S[i] = sum / (i + 1);
    }

    float f = 1.0f / Q[0];
    float denom = 1.0f;
    for (int i = 0; i < N; i++) {
        denom *= (N - i);
        f += S[i] / (denom * Q[i+1]); 
    }

    return std::exp(p) * f;
}

float compute_T(const std::vector<float>& X, const std::vector<float>& Q) {
    int N_plus_1 = X.size();
    int N = N_plus_1 - 1;

    float T_sum = 0.0;

    for (int i = 0; i < N_plus_1; i++) {
        float p = X[i]; //pivot

        std::vector<float> Y;
        for (int j = 0; j < N_plus_1; ++j) {
            if (j != i) {
                Y.push_back(X[j]);
            }
        }

        float f = f_N(p, Y, Q);
        T_sum += f;
    }

    float T = T_sum / N_plus_1;
    return T;
}

