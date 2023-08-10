#pragma once

#include <functional>
#include <map>
#include <iostream>

class TaskScheduler
{
public:
    std::multimap<uint32_t /* time in ms */, std::function<void()>> tasks;

    // void RunAfter(double time, double seconds)
    // {
    //     tasks.insert(time + );
    // }

    // template<typename T>
    // void RunAt(uint32_t time /* in ms */, T&& functor)
    // {
    //     tasks.insert(time, std::forward<T>(functor));
    // }

    void RunAt(uint32_t time /* in ms */, std::function<void()> functor)
    {
        tasks.insert({time, functor});
    }


    void Update(uint32_t time /* in ms */)
    {
        auto it = tasks.begin();
        // for (auto& [functorTime, functor] : tasks)
        while (it != tasks.end())
        {
            if (it->first > time)
                break;

            it->second();

            it = tasks.erase(it);
        }
    }
};