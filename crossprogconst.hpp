#include <SFML/Network.hpp>

#pragma once

enum PacketType {
	CONNECT,
	DISCONNECT,
	CONFIRMATION,
	ITEM,
	VOTE,
	VICTORY,
	RESET,
	BEGIN,
	UPDATE
};

const unsigned short UPORT = 3146;
const unsigned short TPORT = 3147;


struct Data {
	sf::Uint64 id;
	sf::Vector3f pos;
	sf::Vector3f vel;
	sf::Vector3f rot;
	sf::Vector3f mrot;
};

inline sf::Packet& operator <<(sf::Packet& packet, const sf::Vector3f& v) { return packet << v.x << v.y << v.z; }
inline sf::Packet& operator >>(sf::Packet& packet, sf::Vector3f& v) { return packet >> v.x >> v.y >> v.z; }
inline sf::Packet& operator <<(sf::Packet& packet, const Data& d) { return packet << d.id << d.pos << d.vel << d.rot << d.mrot; }
inline sf::Packet& operator >>(sf::Packet& packet, Data& d) { return packet >> d.id >> d.pos >> d.vel >> d.rot >> d.mrot; }