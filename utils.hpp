#pragma once

#include <irrlicht.h>
#include <IrrIMGUI/IrrIMGUI.h>

#include "event.hpp"
#include "network.hpp"
#include "assets.hpp"

using namespace irr;
using namespace irr::core;

// game context
struct Context {
	IrrlichtDevice *device;
	video::IVideoDriver *driver;
	scene::ISceneManager *smgr;
	IrrIMGUI::IIMGUIHandle *imgui;
	scene::ICameraSceneNode *camera;

	EventHandler *events;
	Network *network;
	Assets *assets;

	sf::Clock timer;
};