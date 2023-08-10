#pragma once

#include <vector>
#include <memory>

class Layer
{
public:
	std::vector<std::weak_ptr<Layer>> dependencies;

    Layer() = default;
    virtual ~Layer() = default;
};