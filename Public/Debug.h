#pragma once

#include <fstream>

class Debug
{
public:
    std::ofstream stream;

    Debug()
    {
        stream.open ("output.txt");
    }

    void AddLog(const std::string& msg)
    {
        stream << msg << '\n';
    }

    ~Debug()
    {
        if (stream.is_open())
            stream.close();
    }
};

template<typename T>
Debug& operator<<(Debug& stream, T value)
{
    stream.stream << value;
    return stream;
}

static Debug cout;