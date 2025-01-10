#include "pnoise.h"

PerlinNoise::PerlinNoise() : engine{}, dist{-1.0f, 1.0f} {}

float PerlinNoise::random(int x) {
    engine.seed(static_cast<unsigned int>(x)); // reseed engine based on input
    return dist(engine);
}
 
float PerlinNoise::cos_interpolation(float a, float b, float t) {
    float f = (1 - cosf(PerlinNoise::PI * t)) * 0.5f;
    return a * (1 - f) + b * f;
}

float PerlinNoise::noise(float x) {
    float v1 = random(static_cast<int>(x));
    float v2 = random(static_cast<int>(x) + 1);

    return cos_interpolation(v1, v2, x - static_cast<int>(x));
}

float PerlinNoise::perlinNoise(float x, int octaves, float persistence) {
    float total = 0;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    for (int i = 0; i < octaves; i++) {
        total += noise(x * frequency) * amplitude;
        frequency *= 2.0f;
        amplitude *= persistence;
    }
    return total;
}