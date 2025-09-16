#include "game.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include <csignal>

void sigsegvHandle(int signal) { (void)signal;
	std::cout << signal;
}


Game::Game() : m_networksync(m_context), m_world(m_context, m_networksync), m_player(m_context, m_world, m_networksync), m_interface(m_context, m_player, m_networksync) {

	#ifdef _WIN32
	SetProcessDPIAware();
	#endif

	std::signal(SIGINT, sigsegvHandle);
	std::signal(SIGSEGV, sigsegvHandle);

	// init irrlicht
	SIrrlichtCreationParameters params;
	params.DriverType = video::EDT_OPENGL;
	sf::VideoMode res = sf::VideoMode::getDesktopMode();
	params.WindowSize = core::dimension2d<u32>(res.width, res.height);
	params.Fullscreen = false;
	params.HandleSRGB = true;
	params.Stencilbuffer = true;
	params.Bits = 16;
	params.AntiAlias = 4;
	params.EventReceiver = &m_events;
	params.LoggingLevel = ELL_DEBUG;

	m_device = createDeviceEx(params);
	if(!m_device)
		throw std::runtime_error("Could not create irrlicht device");

	m_device->getLogger()->setLogLevel(irr::ELOG_LEVEL::ELL_NONE);

	// populate context
	m_driver = m_device->getVideoDriver();
	m_smgr = m_device->getSceneManager();
	m_context.device = m_device;
	m_context.driver = m_driver;
	m_context.smgr = m_smgr;
	m_context.events = &m_events;
	m_context.network = &m_network;
	m_context.assets = &m_assets;
	m_context.camera = m_smgr->addCameraSceneNode();
	m_context.camera->setFOV(110/(180/3.14159));
	m_context.smgr->setActiveCamera(m_context.camera);
	//m_context.smgr->setAmbientLight(video::SColor(20,20,20,20));

	// setup and load prerequisites
	m_interface.setup();

	// add callbacks to be loaded
	m_assets.addCallback([&]() {
		m_world.setup();
		return true;
	});
	m_assets.addCallback([&]() {
		m_player.setup();
		return true;
	});

	// mainloop to be called while main thread loads
	m_assets.addMainCallback([&](){
		m_device->yield();
		return true;
	});

	m_assets.load(m_context);
}

Game::~Game() {

}

void Game::mainLoop() try {
	
	bool loaded = false;
	float dt = 0.0f;
	float last = 0.0f;

	while(m_device->run()) {

		// update delta time
		dt = m_device->getTimer()->getTime() - last;
		last = m_device->getTimer()->getTime();
		dt /= 1000.0f;

		m_driver->beginScene(true, true, video::SColor(255, 0x87, 0xce, 0xfa));

		// if(game) do game;
		if(m_interface.getState() == Interface::STATE_GAME) {
			m_networksync.update(dt);
		}

		if(m_interface.getState() != Interface::STATE_LOAD) {
			m_player.update(dt);
			m_world.updateItems(dt, m_player.position());
		}
		

		// check for resets
		if(m_context.network->shouldReset()) {
			m_interface.setState(Interface::State::STATE_MENU);
			m_player.setState(Player::State::STATE_SELECT);
			m_world.reset();
			m_player.reset();
		}



		m_smgr->drawAll();
		m_interface.render();
		m_driver->endScene();
		
		// wait for assets to load
		if(!loaded) {
			if(loaded = m_assets.done(); loaded) {
				m_interface.setState(Interface::STATE_MENU);
			} else if(m_assets.errorFlag()) {
				throw std::runtime_error(m_assets.errorMsg());
			}
		}

		if(m_network.errorFlag()) {
			throw std::runtime_error(m_network.errorMsg());
		}
	}
} catch (std::exception &e) {
	std::cout << e.what() << std::endl;
}