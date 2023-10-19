#ifndef _MIDI_PLAYER_H_
#define _MIDI_PLAYER_H_

#include "AMIDIPlayer.h"

class MIDIPlayer : public AMIDIPlayer
{
public:
    // Begin - ChannelEvents
    virtual void OnNoteOn(int channel, int key, int velocity) override;
    virtual void OnNoteOff(int channel, int key) override;
    virtual void OnProgramChange(int channel, int program) override;
    virtual void OnControlChange(int channel, EControlChange ctrl, int value) override;
    virtual void OnPitchBend(int channel, int value) override;
    // End - ChannelEvents

    // Begin - MetaEvents
    // sf : 0 = key of C, -1 = 1 flat, 1 = 1 sharp
    // mi : major or minor ?
    virtual void OnKeySignature(uint8_t sf, uint8_t mi) {}
    virtual void OnText(const char* text, uint32_t length) {}
    virtual void OnCopyright(const char* copyright, uint32_t length) {}
    virtual void OnTrackName(const char* trackName, uint32_t length) {}
    virtual void OnInstrumentName(const char* instrumentName, uint32_t length) {}
    virtual void OnLyric(const char* lyric, uint32_t length) {}
    virtual void OnMarker(const char* markerName, uint32_t length) {}
    virtual void OnCuePoint(const char* cuePointName, uint32_t length) {}
    // End - MetaEvents
};

#endif // _MIDI_PLAYER_H_