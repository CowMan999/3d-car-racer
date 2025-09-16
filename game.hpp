#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

#include <irrlicht.h>
#include <IrrIMGUI/IrrIMGUI.h>
#include <imgui.h>

#include <iostream>

#include "utils.hpp"
#include "network.hpp"
#include "networksync.hpp"
#include "event.hpp"
#include "interface.hpp"
#include "assets.hpp"
#include "player.hpp"
#include "world.hpp"

using namespace irr;

class Game {

public:
	Game();
	~Game();

	void mainLoop();

private:

	// irrlicht pointers
	IrrlichtDevice *m_device;
	video::IVideoDriver *m_driver;
	scene::ISceneManager *m_smgr;
	IrrIMGUI::IIMGUIHandle *m_imgui;

	Context m_context;
	EventHandler m_events;
	Network m_network;
	NetworkSync m_networksync;
	Assets m_assets;
	World m_world;
	Player m_player;
	Interface m_interface;
};