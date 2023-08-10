#pragma once

#include <vector>

typedef struct _fluid_synth_t fluid_synth_t;                    /**< Synthesizer instance */

struct FullNote
{
    // MIDI channel number (0 to MIDI channel count - 1)
    int channel;

    // ID of a loaded SoundFont
    int fontID;

    // MIDI bank number
    int bankID;

    // MIDI program number
    int presetID;

    // MIDI note number (0-127)
    int key;

    // velocity (0-127, 0=noteoff)
    // The intensity of the note
    int velocity = 110;
};

class SimpleMIDIPlayer
{
public:
    fluid_synth_t* synth;
    
    struct NoteEvent
    {
        float time = 0.f;
        unsigned int fullNoteID = -1; // max since negative
        bool isNoteOnEvent = true; // else, isNotOff
    };
    
    std::vector<FullNote> fullNotes;
    std::vector<NoteEvent> events;

public:
    float time = 0.f;

    void AddNote(const FullNote& note, float timeAdded, float duration = 1.f)
    {
        events.emplace_back(NoteEvent{timeAdded, unsigned int(fullNotes.size()), true});
        events.emplace_back(NoteEvent{timeAdded + duration - 0.001f, unsigned int(fullNotes.size()), false});
        fullNotes.emplace_back(note);
    }

    void Play();
};