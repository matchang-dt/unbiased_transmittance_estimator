#pragma once

#include <cstdint>
#include <limits>
#include "pcg_random.hpp"

class UniformRandom {
public:
    UniformRandom(uint64_t seed, float min, float max)
        : rng(seed), float_min(min), float_max(max), is_float(true) {}

    float next_float() {
        float zero_to_one = static_cast<float>(rng()) / static_cast<float>(pcg32::max());
        return float_min + (float_max - float_min) * zero_to_one;
    }

private:
    pcg32 rng;
    bool is_float;

    float float_min;
    float float_max;
};