#include "random.hpp"

namespace Random {

	static std::random_device rd;
	static std::mt19937 gen(rd());

	int get(int min, int max) {
		std::uniform_int_distribution<> dis(min, max);
		return dis(gen);
	}

	float getf(float min, float max) {
		std::uniform_real_distribution<> dis(min, max);
		return dis(gen);
	}

	bool getb() {
		static int x = 0;
		x = x * 1103515245 + 12345;
		return (x / 65536) % 2;
	}

}