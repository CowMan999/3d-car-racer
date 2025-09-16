#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

#include <irrlicht.h>
#include <fstream>

#include "utils.hpp"
#include "network.hpp"
#include "event.hpp"
#include "assets.hpp"
#include "networksync.hpp"

#pragma once

const float PROJECTILE_SPEED = 200.f;


class World {
public:
	World(Context& context, NetworkSync& networksync);
	~World();

	bool setup();

	scene::IAnimatedMeshSceneNode* getMesh();

	enum WorldName {
		WORLD_FIG8,
		WORLD_ROUND,
		WORLD_COUNT,
	};

	enum Item {
		ITEM_PROJECTILE,
		ITEM_SPIKE,
		ITEM_SPIKESHIELD,
		ITEM_COUNT,
	};

	struct ActiveItem {
		Item type;
		scene::IAnimatedMeshSceneNode* mesh;
		size_t owner;

		struct ItemData {
			vector3df dir;
			float dur = 8;
		} data;
	};

	void loadWorld(WorldName name);
	void hide();
	void show();

	void updateItems(float dt, vector3df playerPos);
	std::vector<ActiveItem>& getItems() {return m_items;}

	void createItem(Item item, vector3df pos, size_t owner, ActiveItem::ItemData data);

	std::vector<scene::IAnimatedMeshSceneNode*>& getItemBoxes() {return m_itemboxes;}

	std::vector<std::pair<vector3df, float>>& checkpoints() {return m_checkpoints;};

	void reset();

private:
	Context& m_context;
	NetworkSync& m_networksync;

	scene::ISceneNode* m_root = nullptr;
	scene::IAnimatedMeshSceneNode* m_mesh = nullptr;

	std::vector<scene::IAnimatedMeshSceneNode*> m_itemboxes;
	std::vector<ActiveItem> m_items;
	std::vector<std::pair<vector3df, float>> m_checkpoints;

	std::vector<scene::ISceneNode*> m_nodes;
};