#include <SFML/Network.hpp>
#include <thread>
#include <deque>
#include <mutex>
#include <atomic>

#include "crossprogconst.hpp"
#include "assets.hpp"

class Network {
public:
	Network();
	~Network();

	struct Client {
		size_t id;
		sf::Uint32 mesh;
		sf::Uint32 tex;
		std::string name;
	};

	void mainLoop();
	void setup(std::string& ip, size_t port, std::string name, Assets::Mesh mesh, Assets::Texture tex);

	void updateData(Data& data);
	const std::deque<Data>& serverData();

	bool sendUdp(sf::Packet& packet);
	bool receiveUdp(sf::Packet& packet);
	bool sendTcp(sf::Packet& packet);
	bool receiveTcp(sf::Packet& packet);
	bool connected();
	bool connecting();
	bool serverUpdate(); 
	std::pair<bool, Client> newClient();
	std::pair<bool, size_t> removedClient();

	size_t id();

	bool errorFlag();
	std::string& errorMsg();
	bool gameBegun();
	bool gameOver();
	size_t placement();
	bool shouldReset();

	std::vector<sf::Packet>& getNewItems() {return m_newitems;}
	sf::Uint32 getMap() {return m_map;}

	Client getClient(size_t id);

private:
	sf::UdpSocket m_usocket;
	sf::TcpSocket m_tsocket;
	size_t m_id;

	std::mutex m_datamutex;
	std::thread* m_thread;

	std::atomic_bool m_errorflag = ATOMIC_VAR_INIT(false);
	std::string m_errorstring;

	Data m_data;
	std::deque<Data> m_serverdata;

	std::string m_serveripstr;
	sf::IpAddress m_serverip;

	std::atomic_bool m_connected = ATOMIC_VAR_INIT(false);
	std::atomic_bool m_connecting = ATOMIC_VAR_INIT(false);
	std::atomic_bool m_serverupdate = ATOMIC_VAR_INIT(false);

	std::mutex m_clientmodmutex;
	std::vector<Client> m_newclients;
	std::map<size_t, Client> m_clients;
	std::vector<size_t> m_removedclients;
	std::atomic_bool m_gamebegun = ATOMIC_VAR_INIT(false);
	std::atomic_bool m_gameover = ATOMIC_VAR_INIT(false);
	std::atomic<size_t> m_placement = ATOMIC_VAR_INIT(0);
	std::atomic_bool m_reset = ATOMIC_VAR_INIT(false);

	std::vector<sf::Packet> m_newitems;

	sf::Uint32 m_map = 0;
	std::string m_name;
	Assets::Mesh m_mesh;
	Assets::Texture m_tex;

};

#pragma once