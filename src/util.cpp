#include "util.h"
#include <random>
#include <ctime>

namespace util {
    auto GetRandomInt(const int min, const int max) -> int {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }

    auto GetRandomFloat(const float min, const float max) -> float {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        std::uniform_real_distribution dist(min, max);
        return dist(gen);
    }
}