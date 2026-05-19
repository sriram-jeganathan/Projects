#pragma once
#include <random>

namespace Random
{
    inline int get(int min, int max)
    {
        static std::mt19937 mt{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(min, max);
        return dist(mt);
    }
}
