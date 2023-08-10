#pragma once

#include "Layer.h"
#include <cassert>

class IntensityLayer : public Layer
{
public:
    struct IntensityData
    {
        // From 0 to 1
        float intensity = 0.f;
    };

    // the interval in second between each value of intensity
    float interval = 0.1f;
    float startTime = 0.f;

    // TODO : store by intensity or by intensity derivative?
    std::vector<IntensityData> intensities;

    void Compute(float time, float from, float to)
    {
        assert(interval != 0.f);

        float currentTime = time;

        while (from + intensities.size() * interval < to)
        {
            IntensityData v;
            v.intensity = (sin(currentTime) + 1) / 2;
            intensities.emplace_back(v);
            currentTime += interval;
        }
    }

    float GetIntensity(float time)
    {
        // time = 2
        // startTime = 0
        // interval = 0.1
        // then id = 20
        int id = (time - startTime) / interval;
        return intensities[id].intensity;

    }
};






