#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <deque>
#include <mutex>

#include "../crossprogconst.hpp"

struct Client {
	std::string name;
	sf::IpAddress ip;
	sf::TcpSocket* tcp;
	size_t id;
	size_t uport = 0;
	size_t tport;
	sf::Uint32 mesh;
	sf::Uint32 tex;

	Data data;
};


extern std::vector<Client> clients;
extern size_t identifier;
extern std::mutex clientmutex;

#pragma once