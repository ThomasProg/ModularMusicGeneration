#pragma once

class Metronome
{
    // Beats per second
    float bps = 60 / 90;

public:
    float time = 0.f;

    operator float() const
    {
        return time;
    }

    void SetBPM(float BPM)
    {
        bps = 60 / BPM;
    }

    void Next()
    {
        time += bps;
    }

    void AddQuarter()
    {
        time += bps * 0.25f;
    }
};