#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <filesystem>

#include "crossprogconst.hpp"
#include "game.hpp"

int main(int argc, char* argv[]) {

	try {
		// cd to executable directory
		std::filesystem::current_path(std::filesystem::path(argv[0]).parent_path());
		(void)argc;
	} catch(std::exception& e) {
		std::cerr << "Could not change working directory: " << e.what() << "\n";
		return 1;
	}

	Game game;
	game.mainLoop();

	return 0;
}