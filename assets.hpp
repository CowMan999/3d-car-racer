#include <IrrIMGUI/IrrIMGUI.h>
#include <irrlicht.h>
#include <IrrAssimp.h>

#include <string>
#include <thread>
#include <atomic>
#include <array>
#include <filesystem>
#include <functional>

#pragma once

using namespace irr;

class Context;

class Assets {
public:
	Assets();
	~Assets();

	void addCallback(std::function<bool()> func);
	void addMainCallback(std::function<bool()> func);

	void load(Context& c);
	bool done();
	bool errorFlag();
	std::string errorMsg();

	enum Texture {
		TEXTURE_CARWARM,
		TEXTURE_CARCOLD,
		TEXTURE_CARPURP,
		TEXTURE_CARGREEN,
		TEXTURE_CARRAINBOW,
		TEXTURE_COUNT
	};

	enum Mesh {
		MESH_CAR_PIXEL,
		MESH_CAR_MODERN,
		MESH_CAR_RING,
		MESH_CAR_BLOB,
		MESH_CAR_SHIP,
		MESH_CAR_JAKE,

		MESH_WORLD_FIG8,
		MESH_WORLD_ROUND,
		
		MESH_ITEMBOX,
		MESH_PROJECTILE,
		MESH_SPIKE,
		MESH_COUNT
	};

	enum GuiTexture {
		GUI_TEXTURE_TEST,
		GUI_TEXTURE_COUNT
	};

	video::ITexture* getTexture(Texture t);
	scene::IAnimatedMesh* getMesh(Mesh m);
	IrrIMGUI::IGUITexture* getGuiTexture(GuiTexture g);


private:
	std::array<video::ITexture*, TEXTURE_COUNT> m_textures = {nullptr};
	std::array<scene::IAnimatedMesh*, MESH_COUNT> m_meshes = {nullptr};
	std::array<IrrIMGUI::IGUITexture*, GUI_TEXTURE_COUNT> m_guitextures = {nullptr};

	std::array<const char*, TEXTURE_COUNT> m_texturenames;
	std::array<const char*, MESH_COUNT> m_meshnames;
	std::array<const char*, GUI_TEXTURE_COUNT> m_guitexturenames;

	std::vector<std::function<bool()>> m_functions;
	std::function<bool()> m_mfunction;

	std::atomic_bool m_loaded = ATOMIC_VAR_INIT(false);
	std::atomic_bool m_errorflag = ATOMIC_VAR_INIT(false);
	std::string m_errorstring;

	IrrlichtDevice* m_device = nullptr;
	IrrIMGUI::IIMGUIHandle* m_imgui = nullptr;

	std::thread* m_thread = nullptr;
};