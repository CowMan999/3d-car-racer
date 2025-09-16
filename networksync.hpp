#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <deque>
#include <irrlicht.h>
#include <IrrIMGUI/IrrIMGUI.h>

#include "crossprogconst.hpp"

#include <irrlicht.h>
#include <iostream>

#include "utils.hpp"
#include "network.hpp"
#include "event.hpp"
#include "assets.hpp"

#pragma once

using namespace irr;

class NetworkSync {
public:
	NetworkSync(Context& context);
	~NetworkSync();
	void update(float dt);

	std::map<size_t, std::pair<scene::ISceneNode*, scene::IAnimatedMeshSceneNode*>> getPlayers() {return m_players;}
	
private:
	Context& m_context;
	std::map<size_t, std::pair<scene::ISceneNode*, scene::IAnimatedMeshSceneNode*>> m_players;
	std::map<size_t, vector3df> m_playervel;
};