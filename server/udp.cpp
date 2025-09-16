#include "udp.hpp"

void udplisten() {
	sf::UdpSocket listener;
	sf::Clock clock;

	// check for new connections
	if(listener.bind(UPORT) != sf::Socket::Done) {
		abort();
	}

	listener.setBlocking(false);

	std::cout << "bound UDP socket to port " << UPORT << "\n";

	while(true) {
		sf::Packet packet;
		sf::IpAddress sender;
		unsigned short port;
		sf::Socket::Status s = listener.receive(packet, sender, port);
		if(s == sf::Socket::Done) {

			int type; packet >> type;

			// find client
			clientmutex.lock();
			sf::Uint64 id; packet >> id; size_t cli = -1;
			for(size_t i = 0; i < clients.size(); i++) {
				if(clients[i].id == id) {
					cli = i;
					if(clients[i].uport == 0) {
						clients[i].uport = port;
					}
					break;
				}
			}
			clientmutex.unlock();

			if(cli == (size_t)-1) {
				std::cout << "recieved message from unregistered client " << id << "\n";
				continue;
			}

			switch (type) {
			
			case UPDATE: {
				clientmutex.lock();
				packet >> clients[cli].data;
				clientmutex.unlock();
				break;
			}

			default:
				std::cout << "received unknown packet type " << type << "\n";
				break;

			}
		}

		if(clock.getElapsedTime().asMilliseconds() > 20) {
			clock.restart();
			clientmutex.lock();

			sf::Packet sendpacket;
			sendpacket << UPDATE << (sf::Uint64)clients.size();
			for (size_t i = 0; i < clients.size(); i++) {
				sendpacket << clients[i].data;
			}

			for (size_t i = 0; i < clients.size(); i++) {
				if(clients[i].uport == 0) continue;
				listener.send(sendpacket, clients[i].ip, clients[i].uport);
			}
			clientmutex.unlock();
		}

		//static sf::Clock c;
		//if(c.getElapsedTime().asSeconds() > 1) {
		//	c.restart();
		//	clientmutex.lock();
		//	std::cout << "clients: " << clients.size() << "\n";
		//	for(size_t i = 0; i < clients.size(); i++) {
		//		std::cout << "client " << i << ": " << clients[i].id << " " << clients[i].ip << " " << clients[i].uport << "\n";
		//		std::cout << "data: pos:" << clients[i].data.pos.x << ',' << clients[i].data.pos.y << ',' << clients[i].data.pos.z << "\n";
		//		std::cout << "data: vel:" << clients[i].data.vel.x << ',' << clients[i].data.vel.y << ',' << clients[i].data.vel.z << "\n";
		//		std::cout << "data: rot:" << clients[i].data.rot.x << ',' << clients[i].data.rot.y << ',' << clients[i].data.rot.z << "\n";
		//		std::cout << "data: mrot:" << clients[i].data.mrot.x << ',' << clients[i].data.mrot.y << ',' << clients[i].data.mrot.z << "\n\n";
		//	}
		//	clientmutex.unlock();
		//}

	}
	
}