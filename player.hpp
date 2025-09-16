#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

#include <irrlicht.h>
#include <iostream>
#include <bitset>

#include "utils.hpp"
#include "network.hpp"
#include "event.hpp"
#include "assets.hpp"
#include "world.hpp"
#include "random.hpp"
#include "networksync.hpp"

// constants
const float GRAVITY = 40.f;
const float FORWARD_SPEED = 100.f;
const float DRIFT_TURBO_SPEED = 44.f;
const float ROTATIONAL_SPEED = 320.f;
const float DRIFT_ROTATIONAL_SPEED = 180.f;
const float ROTATIONAL_SPEED_CAP = 50.f;
const float ROTATIONAL_SPEED_DRIFT = 1.6f;
const float ROTATIONAL_SPEED_ROLL_CAP = 660.f;
const float DRAG = 0.75f;
const float ROTATIONAL_DRAG = 0.95f;
const float X_RAY_OFFSET = 1.0f;
const float Z_RAY_OFFSET = 3.0f;
const float HOVER_HEIGHT = 3.4f;
const float HOVER_FORCE = 70.0f;
const float CAMERA_OFFSET_VERT = 2.05f;
const float CAMERA_OFFSET_BACK = 8.0f;
const float CAMERA_SHAKE = 0.0275f;
const float HOVER_ROTATIONAL_FORCE = 320.0f;
const float HOVER_ROTATIONAL_MAX = 35.0f;
const float HOVER_FORCE_MAX = 55.0f;
const float CANCEL_GRAV_FORCE = 50.0f;
const int TOTAL_LAPS = 5;

class Player {

public:
	Player(Context& context, World& world, NetworkSync& networksync);
	~Player();

	bool setup();

	void update(float dt);

	enum State {
		STATE_PLAYING,
		STATE_SELECT,
 
		STATE_COUNT
	};

	void setState(State state);
	void selectCar(std::pair<std::bitset<4>, std::bitset<4>> bits);
	void selectTexture(std::pair<std::bitset<4>, std::bitset<4>> bits);

	float countdown() {return m_gamecountdown.getElapsedTime().asSeconds();};
	float speed();
	float nitro();
	size_t checkpoint();
	size_t laps();
	bool raceComplete() {return m_laps >= TOTAL_LAPS;}
	vector3df position() {return m_root->getPosition();}

	bool hasItem() {return m_item != World::Item::ITEM_COUNT;}
	World::Item item() {return m_item;}

	Assets::Mesh& getMeshEnum() {return m_meshEnum;}
	Assets::Texture& getTexEnum() {return m_texEnum;}

	World& getWorld() {return m_world;}

	void reset() {m_laps = 0; m_checkpoint = 0;}

private:
	Context& m_context;
	World& m_world;
	NetworkSync& m_networksync;

	scene::ISceneNode* m_root = nullptr;
	scene::IAnimatedMeshSceneNode* m_mesh = nullptr;
	scene::ILightSceneNode* m_light = nullptr;
	scene::ISceneNodeAnimatorCollisionResponse *m_collision = nullptr;
	scene::IShadowVolumeSceneNode* m_shadow = nullptr;
	vector3df m_camerapos = vector3df(0.0f, 0.0f, 0.0f);

	video::ITexture* m_texture = nullptr;

	vector3df m_velocity = vector3df(0.0f, 0.0f, 0.0f);
	vector3df m_rotationalvelocity = vector3df(0.0f, 0.0f, 0.0f);
	vector3df m_rotationalvelocitymesh = vector3df(0.0f, 0.0f, 0.0f);
	vector3df m_lastsafepos = vector3df(0.0f, 0.0f, 0.0f);

	bool m_drifting = false;
	bool m_driftingcancel = false;
	int m_driftingdir = 0;
	bool m_drifted = false;
	bool m_flying = false;
	float m_flyrotation = INFINITY;
	sf::Clock m_driftclock;
	sf::Clock m_driftcancelclock;

	State m_state = STATE_SELECT;

	sf::Clock m_gamecountdown;
	float m_nitro = 0;

	std::vector<std::pair<vector3df, float>>* m_checkpoints;
	size_t m_checkpoint = 0;
	size_t m_laps = 0;

	Assets::Mesh m_meshEnum;
	Assets::Texture m_texEnum;

	World::Item m_item = World::Item::ITEM_COUNT;
};

#pragma once