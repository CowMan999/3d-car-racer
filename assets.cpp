#include "assets.hpp"
#include "utils.hpp"

Assets::Assets() {
	m_texturenames[TEXTURE_CARWARM] = "./assets/textures/carwarm.png";
	m_texturenames[TEXTURE_CARCOLD] = "./assets/textures/carcold.png";
	m_texturenames[TEXTURE_CARPURP] = "./assets/textures/carpurple.png";
	m_texturenames[TEXTURE_CARGREEN] = "./assets/textures/cargreen.png";
	m_texturenames[TEXTURE_CARRAINBOW] = "./assets/textures/carrainbow.png";

	m_meshnames[MESH_CAR_PIXEL] = "./assets/meshes/carpixel.glb";
	m_meshnames[MESH_CAR_MODERN] = "./assets/meshes/carmodern.glb";
	m_meshnames[MESH_CAR_RING] = "./assets/meshes/carring.glb";
	m_meshnames[MESH_CAR_BLOB] = "./assets/meshes/carblob.glb";

	m_meshnames[MESH_CAR_SHIP] = "./assets/meshes/ship.fbx";
	m_meshnames[MESH_CAR_JAKE] = "./assets/meshes/jake.fbx";

	m_meshnames[MESH_WORLD_FIG8] = "./assets/meshes/track1.glb";
	m_meshnames[MESH_WORLD_ROUND] = "./assets/meshes/track2.glb";

	m_meshnames[MESH_ITEMBOX] = "./assets/meshes/itembox.fbx";
	m_meshnames[MESH_PROJECTILE] = "./assets/meshes/projectile.glb";
	m_meshnames[MESH_SPIKE] = "./assets/meshes/spike.glb";
	
	m_guitexturenames[GUI_TEXTURE_TEST] = "./assets/guitextures/play.png";
}

Assets::~Assets() {
	for(auto& i : m_textures) {
		if(i != nullptr)
			i->drop();
	}

	for(auto& i : m_meshes) {
		if(i != nullptr)
			i->drop();
	}

	for(auto& i : m_guitextures) {
		if(m_imgui != nullptr) {
			if(i != nullptr)
				m_imgui->deleteTexture(i);
		} else break;
	}

	if(m_thread != nullptr) {
		m_thread->join();
		delete m_thread;
	}
}
Context * cc;
void Assets::load(Context& c) {
	cc = &c;
	m_thread = new std::thread([&]() {

		// add irrassimp loader
		c.smgr->addExternalMeshLoader(new IrrAssimpImport(c.smgr));

		// load meshes
		for(size_t i = 0; i < MESH_COUNT; i++) {
			m_meshes[i] = c.smgr->getMesh(m_meshnames[i]);

			if(m_meshes[i] == nullptr) {
				m_errorstring = "Could not load mesh: ";
				m_errorstring += m_meshnames[i];
				m_errorflag = true;
				return;
			}
		}

		// load gui textures
		for(size_t i = 0; i < GUI_TEXTURE_COUNT; i++) {
			video::IImage* img = c.driver->createImageFromFile(m_guitexturenames[i]);
			if(img == nullptr) {
				m_errorstring = "Could not load gui texture: ";
				m_errorstring += m_guitexturenames[i];
				m_errorflag = true;
				return;
			}
			m_guitextures[i] = c.imgui->createTexture(img);
			if(m_guitextures[i] == nullptr) {
				m_errorstring = "Could not create sutable gui texture: ";
				m_errorstring += m_guitexturenames[i];
				m_errorflag = true;
				return;
			}
			img->drop();
		}

		m_loaded = true;

	});
}

bool Assets::done() {
	bool d = m_loaded;
	if(d) {
		m_thread->join();
		delete m_thread;

		// load textures
		for(size_t i = 0; i < TEXTURE_COUNT; i++) {
			m_textures[i] = cc->driver->getTexture(m_texturenames[i]);
			if(m_textures[i] == nullptr) {
				throw std::runtime_error(std::string("Could not load texture: ")+m_texturenames[i]);
			}
		}

		// call callbacks
		for(auto& i : m_functions) {
			if(!i()) {
				throw std::runtime_error("callback failed");
			}
		}
	}
	return m_loaded;
}

bool Assets::errorFlag() {
	return m_errorflag;
}

std::string Assets::errorMsg() {
	return m_errorstring;
}

video::ITexture* Assets::getTexture(Assets::Texture t) {
	return m_textures[t];
}

scene::IAnimatedMesh* Assets::getMesh(Assets::Mesh m) {
	return m_meshes[m];
}

IrrIMGUI::IGUITexture* Assets::getGuiTexture(Assets::GuiTexture g) {
	return m_guitextures[g];
}

void Assets::addCallback(std::function<bool()> f) {
	m_functions.push_back(f);
}

void Assets::addMainCallback(std::function<bool()> f) {
	m_mfunction = f;
}