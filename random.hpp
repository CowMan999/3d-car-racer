#include <random>

#pragma once

namespace Random {

	// get random int between min and max
	int get(int min, int max);

	// get random float between min and max
	float getf(float min, float max);

	// get random bool
	bool getb();
}