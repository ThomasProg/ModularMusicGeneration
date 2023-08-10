#include "SimpleMIDIPlayer.h"
#include <algorithm>
#include <fluidsynth.h>
#include <chrono>
#include <thread>
#include <iostream>

void SimpleMIDIPlayer::Play()
{
    std::sort(events.begin(), events.end(), [](const NoteEvent& lhs, const NoteEvent& rhs)
    {
        return lhs.time < rhs.time;
    });

    int i = 0;
    for (; i < events.size() && this->time >= events[i].time; i++);

    for (; i < events.size(); i++)
    {
        const NoteEvent& event = events[i];
        const FullNote& note = fullNotes[event.fullNoteID];
        fluid_synth_program_select(synth, note.channel, note.fontID, note.bankID, note.presetID);

        if (event.isNoteOnEvent)
        {
            fluid_synth_noteon(synth, note.channel, note.key, note.velocity); 
        }
        else 
        {
            fluid_synth_noteoff(synth, note.channel, note.key);
        }

        if (i + 1 < events.size())
        {
            // std::cout << int((events[i + 1].time - events[i].time) * 1000) << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(int((events[i + 1].time - events[i].time) * 1000)));
        }
    }
}