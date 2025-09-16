#include "tcp.hpp"

void tcplisten() {
	sf::TcpListener listener;
	std::mutex tcpmutex;
	sf::SocketSelector selector;
	std::vector<unsigned int> maps;
	std::vector<size_t> victors;
	time_t victorytime = 0;

	std::thread listenerthread([&](){
		if(listener.listen(TPORT) != sf::Socket::Done) {
			abort();
		}

		std::cout << "bound TCP socket to port " << TPORT << "\n";

		while(true) {
			sf::TcpSocket* socket = new sf::TcpSocket;
			if(listener.accept(*socket) == sf::Socket::Done) {
				clientmutex.lock();
				tcpmutex.lock();
				clients.push_back(Client());
				clients.back().tcp = socket;
				clients.back().id = identifier++;
				clients.back().ip = socket->getRemoteAddress();
				clients.back().tport = socket->getRemotePort();
				selector.add(*socket);
				tcpmutex.unlock();
				clientmutex.unlock();
				std::cout << "establishing connection with " << clients.back().ip.toString() << "\n";
			} else {
				delete socket;
			}
		}
	});

	while(true) {
		if(selector.wait(sf::milliseconds(100))) {
			tcpmutex.lock();
			for(size_t i = 0; i < clients.size(); i++) {
				if(selector.isReady(*clients[i].tcp)) {
					sf::Packet packet;
					if(sf::Socket::Status s = clients[i].tcp->receive(packet); s == sf::Socket::Done) {
						int type; packet >> type;

						switch (type)
						{
						case CONNECT: {
							clientmutex.lock();
							packet >> clients[i].mesh;
							packet >> clients[i].tex;
							packet >> clients[i].name;
							sf::Packet sendpacket;
							std::cout << "client " << ' ' << clients[i].id << ' ' << clients[i].mesh << ' ' << clients[i].tex << ' ' << clients[i].name << " connected\n";
							sendpacket << CONFIRMATION << (sf::Uint64)clients[i].id;

							// let new client know about all other clients
							int size = 0;
							for(size_t j = 0; j < clients.size(); j++) {
								if(j == i) continue;
								size++;
							}
							sendpacket << (sf::Uint64)size;
							for(size_t j = 0; j < clients.size(); j++) {
								if(j == i) continue;
								sendpacket << (sf::Uint64)clients[j].id << clients[j].mesh << clients[j].tex << clients[j].name;
							}
							clientmutex.unlock();

							clients[i].tcp->send(sendpacket);

							// let all other clients know about the new client
							for(size_t j = 0; j < clients.size(); j++) {
								if(j == i) continue;
								sf::Packet spacket;
								spacket << CONNECT << (sf::Uint64)clients[i].id << clients[i].mesh << clients[i].tex << clients[i].name;
								clients[j].tcp->send(spacket);
							}

						} break;
						
						case VOTE: {
							sf::Uint32 map = -1;
							packet >> map;
							std::cout << "player " << clients[i].id << " voted map " << map << '\n';
							if(map != (sf::Uint32)-1) {
								maps.push_back(map);
							}

						} break;

						case ITEM: {
							
							// recive item info
							sf::Uint32 item;
							float posX, posY, posZ, dirX, dirY, dirZ, dur;
							packet >> item >> posX >> posY >> posZ >> dirX >> dirY >> dirZ >> dur;

							// repopulate p with the same shit
							sf::Packet p;
							sf::Uint64 iid = clients[i].id;
							p << ITEM << item << iid << posX << posY << posZ << dirX << dirY << dirZ << dur;

							// let all clients item has spawned
							for(size_t j = 0; j < clients.size(); j++) {
								clients[j].tcp->send(p);
							}

						} break;

						case BEGIN: {
							if(maps.empty()) break;
							sf::Uint32 map = maps[rand()%maps.size()];
							sf::Packet p; p << BEGIN << map;
							std::cout << "map selected " << map << '\n';

							// let all clients know game started
							for(size_t j = 0; j < clients.size(); j++) {
								clients[j].tcp->send(p);
							}

							maps.clear();
						} break;

						case VICTORY: {
							victors.push_back(clients[i].id);
							std::cout << "client " << clients[i].id << " completed the track!\n";
							
							// check if all players completed track
							bool all = true;
							for(auto& client : clients) {
								if(std::find(victors.begin(), victors.end(), client.id) == victors.end()) {
									all = false;
								}
								if(!all) {
									break;
								}
							}

							if(all) {
								std::cout << "race complete!\n";
								for(size_t j = 0; j < clients.size(); j++) {
									sf::Packet p; p << VICTORY << (sf::Uint32)std::distance(victors.begin(), std::find(victors.begin(), victors.end(), clients[j].id));
									clients[j].tcp->send(p);
								}
								victors.clear();
								victorytime = time(nullptr)+4;
							}
						} break;


						default:
							break;
						}

					} else if(s == sf::Socket::Disconnected) {
						clientmutex.lock();
						sf::Uint64 id = clients[i].id;
						std::cout << "client " << clients[i].name << " disconnected\n";
						selector.remove(*clients[i].tcp);
						delete clients[i].tcp;
						clients.erase(clients.begin() + i); // full removal of client
						clientmutex.unlock();

						// let all other clients know about the disconnected client
						for(size_t j = 0; j < clients.size(); j++) {
							sf::Packet sendpacket;
							sendpacket << DISCONNECT << id;
							clients[j].tcp->send(sendpacket);
						}
					}
				}
			}
			tcpmutex.unlock();
		}

		if(victorytime < time(nullptr) && victorytime != 0) {
			tcpmutex.lock();
			victorytime = 0;
			for(size_t j = 0; j < clients.size(); j++) {
				sf::Packet sendpacket;
				sendpacket << RESET;
				clients[j].tcp->send(sendpacket);
			}
			tcpmutex.unlock();
		}
	}

	listenerthread.join();
}