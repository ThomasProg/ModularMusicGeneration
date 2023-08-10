#pragma once

#include "ModularMusicGenerator.h"

// class SimpleBeatLayer : public Layer
// {
// public:
// 	virtual void GetNotes(std::vector<Note>& outNotes, float startTime, float endTime) override
//     {
//         startTime = ceil(startTime);
//         endTime = ceil(endTime);

//         int n = 1;

//         // 120 bpm
//         float delay = 60.f / 120.f;

//         while (startTime < endTime)
//         {
//             Note note;
//             note.duration = delay;
//             note.note = 2;
//             note.velocity = 100;

//             outNotes.push_back(note);

//             startTime += delay;
//             n++;
//         }
//     }
// };