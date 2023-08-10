#pragma once

#include <vector>
#include <memory>

class Layer;

class LayerManager
{
	// Vertical layers
	// Sorted in depdency order
	std::vector<std::shared_ptr<Layer>> layers;

public:

	// /**
	//  * \return Returns the notes that have to be played before currentTime+duration
	// */
	// std::vector<Note> GetFollowingNotes(float duration);

	void AddLayer(const std::shared_ptr<Layer>& layer)
	{
		layers.push_back(layer);
	}
};