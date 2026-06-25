#include <cmath>
#include "easing_functions.hpp"

//https://easings.net/
float pi = 3.14159265;
namespace Easings{
    float EaseOutBack(float x) {
            const float c1 = 1.70158f;
            const float c3 = c1 + 1.0f;
            return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
    }

    float easeInSine(float x){
        return 1 - std::cos((x * pi) / 2);
    }

    float EaseInOutCubic(float x) {
        return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f;
    }
}