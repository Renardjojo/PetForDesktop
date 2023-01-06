#pragma once

#include <vector>
#include <algorithm>

class UtilitySystem
{
public:
    struct Need
    {
        int   id; // Could be enum, hash or index in buffer
        float value;
        float min;
        float max;
        float minThreshold;
        float maxThreshold;

        float ratioInThreshold()
        {
            return std::clamp((value - minThreshold) / (maxThreshold - minThreshold), 0.f, 1.f);
        }

        float ratio()
        {
            return (value - min) / (max - min);
        }

        void add(float count)
        {
            value = std::min(max, value + count);
        }

        void reduce(float count)
        {
            value = std::max(min, value - count);
        }
    };

public:
    std::vector<Need> needs;

public:

    void addNeed(float value, float min, float max, float minThreshold, float maxThreshold)
    {
        needs.emplace_back(needs.size(), value, min, max, minThreshold, maxThreshold);
    }

    int getPriority()
    {
        Need* pCurrent               = nullptr;
        float currentHighestPriority = 1.f;

        for (size_t i = 0; i < needs.size(); i++)
        {
            float currentRatio = needs[i].ratioInThreshold();
            if (currentRatio < currentHighestPriority)
            {
                currentHighestPriority = currentRatio;
                pCurrent = &needs[i];
            }
        }

        return pCurrent == nullptr ? -1 : pCurrent->id;
    }
};