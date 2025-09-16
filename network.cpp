#include "network.hpp"
#include <iostream>

Network::Network() {
}

Network::~Network() {
	m_usocket.unbind();
	m_tsocket.disconnect();
}

void Network::setup(std::string& ip, size_t port, std::string name, Assets::Mesh mesh, Assets::Texture tex) {
	if(m_usocket.bind(port) != sf::Socket::Done) {
		throw std::runtime_error("Could not bind socket");
	}

	m_usocket.setBlocking(false);

	m_serveripstr = ip;
	m_thread = new std::thread(&Network::mainLoop, this);

	m_name = name;
	m_mesh = mesh;
	m_tex = tex;
}

void Network::mainLoop() {
	try {
		m_connecting = true;
		m_serverip = sf::IpAddress(m_serveripstr);
		
		// attempt connection
		sf::Socket::Status s = m_tsocket.connect(m_serverip, TPORT, sf::seconds(5));
		if(s != sf::Socket::Done) {
			throw std::runtime_error("Could not connect to server at " + m_serveripstr + ":" + std::to_string(TPORT));
		}

		m_tsocket.setBlocking(false);

		// populate connection packet with info about my client
		sf::Packet initp;
		std::cout << (sf::Uint32)m_mesh << ' ' << (sf::Uint32)m_tex;
		initp << CONNECT << (sf::Uint32)m_mesh << (sf::Uint32)m_tex << m_name;

		// send connection packet
		if(!sendTcp(initp)) {
			throw std::runtime_error("Could not send connection packet");
		}

		// receive confirmation packet and set id
		sf::Packet confp;
		sf::Clock clock;
		while(!receiveTcp(confp)) {
			if(clock.getElapsedTime().asSeconds() > 5)
				throw std::runtime_error("Could not receive confirmation packet");
		}

		int type;
		confp >> type;
		if(type != CONFIRMATION) {
			throw std::runtime_error("Received wrong confirmation packet type");
		} else {
			confp >> (sf::Uint64&)m_id;
			std::cout << "received id " << m_id << "\n";
		}

		// receive all other clients
		m_clientmodmutex.lock();
		sf::Uint64 size = 0;
		confp >> size;
		for(size_t i = 0; i < size; i++) {
			sf::Uint64 id; sf::Uint32 mesh; sf::Uint32 tex; std::string name;
			confp >> id >> mesh >> tex >> name;
			std::cout << "received client " << id << " " << name << "\n";
			m_newclients.push_back({id, mesh, tex, name});
		}
		m_clientmodmutex.unlock();

		m_connected = true;
		m_connecting = false;

		bool recievedudp = true;
		while(true) {
			if(recievedudp) {
				sf::Packet packet;
				m_datamutex.lock();
				packet << UPDATE << (sf::Uint64)m_id << m_data;
				m_datamutex.unlock();
				sendUdp(packet);

				recievedudp = false;
			}

			if(!recievedudp) {
				sf::Packet packet;
				if(receiveUdp(packet)) {
					recievedudp = true;

					m_datamutex.lock();

					int type; packet >> type;
					if(type != UPDATE) {
						throw std::runtime_error("Received wrong packet type via udp socket");
					}

					m_serverupdate = true;


					sf::Uint64 size; packet >> size;
					if(size != m_serverdata.size()) {
						m_serverdata.resize(size);
					}

					for(size_t i = 0; i < size; i++) {
						packet >> m_serverdata[i];
					}
					m_datamutex.unlock();
				}
			}

			// receive tcp packets, always check cause these can't be missed
			sf::Packet packet;
			if(receiveTcp(packet)) {
				int type; packet >> type;
				if(type == CONNECT) {
					sf::Uint64 id; sf::Uint32 mesh; sf::Uint32 tex; std::string name;
					packet >> id >> mesh >> tex >> name;
					if(id == m_id) {
						throw std::runtime_error("Received wrong connect packet type via tcp socket");
					}
					m_clientmodmutex.lock();
					m_newclients.push_back({id, mesh, tex, name});
					m_clientmodmutex.unlock();
				} else if(type == DISCONNECT) {
					sf::Uint64 id; packet >> id;
					if(id == m_id) {
						throw std::runtime_error("Received wrong disconnect packet type via tcp socket");
					}
					m_clientmodmutex.lock();
					m_removedclients.push_back(id);
					m_clientmodmutex.unlock();
				} else if(type == BEGIN) {
					m_gamebegun = true;
					packet >> m_map;
				} else if(type == VICTORY) {
					m_gameover = true;
					sf::Uint32 placement;
					packet >> placement;
					m_placement = placement;
				} else if(type == ITEM) {
					m_newitems.push_back(packet);
				} else if(type == RESET) {
					m_reset = true;
					m_newitems.clear();
					m_gameover = false;
					m_placement = 0;
					m_gamebegun = false;
					m_map = 0;
				}
			}
		}
		

	} catch(std::exception& e) {
		m_errorstring = e.what();
		m_errorflag = true;
	}
}


// dont forget to stick the id into the packets retard
bool Network::sendUdp(sf::Packet& packet) {
	sf::Socket::Status s = m_usocket.send(packet, m_serverip, UPORT);
	if(s == sf::Socket::Done) {
		return true;
	} else {
		return false;
	}
}

bool Network::sendTcp(sf::Packet& packet) {
	sf::Socket::Status s = m_tsocket.send(packet);
	
	check:
	if(s == sf::Socket::Done) {
		return true;
	} else if(s == sf::Socket::Disconnected) {
		throw std::runtime_error("not connected to server");
		return false;
	} else if(s == sf::Socket::Partial) {
		while (s == sf::Socket::Partial) {
			s = m_tsocket.send(packet);
			goto check; // yeah i know, fuck good practice
		}
		
	}
	return false;
}

bool Network::receiveUdp(sf::Packet& packet) {
	sf::IpAddress sender;
	unsigned short port;
	sf::Socket::Status s = m_usocket.receive(packet, sender, port);
	if(s == sf::Socket::Done) {
		return true;
	} else {
		return false;
	}
}

bool Network::receiveTcp(sf::Packet& packet) {
	sf::Socket::Status s = m_tsocket.receive(packet);
	if(s == sf::Socket::Done) {
		return true;
	} else if(s == sf::Socket::Disconnected) {
		throw std::runtime_error("Disconnected from server");
		return false;
	} else {
		return false;
	}
}

bool Network::errorFlag() {
	return m_errorflag;
}

std::string& Network::errorMsg() {
	return m_errorstring;
}

void Network::updateData(Data& data) {
	m_datamutex.lock();
	m_data = data;
	m_datamutex.unlock();
}

const std::deque<Data>& Network::serverData() {
	return m_serverdata;
}

bool Network::connected() {
	return m_connected;
}

bool Network::connecting() {
	return m_connecting;
}
size_t Network::id() {
	return m_id;
}

std::pair<bool, Network::Client> Network::newClient() {
	m_clientmodmutex.lock();
	bool b = !m_newclients.empty();
	Client c;
	if(b) {
		c = m_newclients.back();
		m_newclients.pop_back();
	}
	m_clientmodmutex.unlock();

	m_clients[c.id] = c;
	return {b, c};
}

std::pair<bool, size_t> Network::removedClient() {
	m_clientmodmutex.lock();
	bool b = !m_removedclients.empty();
	size_t id = 0;
	if(b) {
		id = m_removedclients.back();
		m_removedclients.pop_back();
	}
	m_clientmodmutex.unlock();

	m_clients[id] = {};

	return {b, id};
}

bool Network::serverUpdate() {
	bool b = m_serverupdate;
	m_serverupdate = false;
	return b;
}

bool Network::gameBegun() {
	bool b = m_gamebegun;
	m_gamebegun = false;
	return b;
}

bool Network::shouldReset() {
	bool b = m_reset;
	m_reset = false;
	return b;
}

bool Network::gameOver() {
	return m_gameover;
}

size_t Network::placement() {
	return m_placement;
}

Network::Client Network::getClient(size_t id) {
	return m_clients[id];
}