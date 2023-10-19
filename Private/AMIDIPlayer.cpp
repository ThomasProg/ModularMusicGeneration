#include "AMIDIPlayer.h"

template<typename IT, typename ... ARGS> 
requires std::forward_iterator<IT>
void PlayIt(const IT begin, const IT end, ARGS&& ... args)
{
    for (IT it = begin; begin != end; it++)
    {
        it->second(std::forward<ARGS...>(args...));
    }
}

template<typename IT> 
requires std::forward_iterator<IT>
void PlayIt(const IT begin, const IT end)
{
    for (IT it = begin; begin != end; it++)
    {
        it->second();
    }
}


void AMIDIPlayer::Play(uint32_t from, uint32_t to)
{
    PlayIt(systemEvents.lower_bound(from), systemEvents.lower_bound(to));
    PlayIt(metaEvents.lower_bound(from), metaEvents.lower_bound(to));

    for (size_t i = 0; i < tracks.size(); i++)
    {
        PlayIt(tracks[i].channelEvents.lower_bound(from), tracks[i].channelEvents.lower_bound(to), i);
    }
}

