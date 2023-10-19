#ifndef _AMIDI_PLAYER_H_
#define _AMIDI_PLAYER_H_

#include <map>
#include <functional>
#include "MIDIHelpers.h"


class AMIDIPlayer
{
public:
    struct ChannelEvent
    {
    public:
        virtual void operator()(char8_t channel) {};
    };
    using ChannelEventPtr = ChannelEvent*;

    struct SysEvent
    {
    public:
        virtual void operator()() {};
    };
    using SysEventPtr = SysEvent*;

    struct MetaEvent
    {
    public:
        virtual void operator()() {};
    };
    using MetaEventPtr = MetaEvent*;

    struct NoteOn : public ChannelEvent
    {
    public:
        int key;
        int velocity;
    };

    struct NoteOff : public ChannelEvent
    {
    public:
        int key;
    };

private:
    struct Track
    {
    public:
        std::multimap<uint32_t /* time in ms */, ChannelEvent&> channelEvents;
    };

    std::vector<Track> tracks;
    std::multimap<uint32_t /* time in ms */, SysEvent&> systemEvents;
    std::multimap<uint32_t /* time in ms */, MetaEvent&> metaEvents;

    using ChannelEventsContainer = decltype(Track::channelEvents);
    using SystemEventsContainer = decltype(systemEvents);
    using MetaEventsContainer = decltype(metaEvents);

public:
    // Play all events from "from" to "to" ; in ms
    void Play(uint32_t from, uint32_t to);

    ChannelEvent* (*NoteOn)();
    ChannelEvent* (*NoteOff)();
};

#endif // _MIDI_PLAYER_H_