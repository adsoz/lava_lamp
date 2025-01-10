#include <random>

class PerlinNoise {
    std::mt19937 engine;
    std::uniform_real_distribution<float> dist;
    static constexpr float PI = 3.14159265358979323846f;
    float random(int x);
    float cos_interpolation(float a, float b, float t);
  public:
    PerlinNoise();
    float perlinNoise(float x, int octaves, float persistence);
    float noise(float x);
};