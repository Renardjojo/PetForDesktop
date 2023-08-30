#pragma once

#include <cstdlib>

inline int randNum(int min, int max)
{
    return (rand() % (((max) + 1) - (min))) + (min);
}