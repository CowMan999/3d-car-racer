#include "world.hpp"

World::World(Context& context, NetworkSync& networksync) : m_context(context), m_networksync(networksync) {

}

World::~World() {
	if(m_mesh != nullptr)
		m_mesh->drop();
}

bool World::setup() {


	return true;
}

void World::loadWorld(WorldName name) {
	Assets::Mesh mesh = Assets::MESH_COUNT;
	std::string path;
	switch(name) {
		case WORLD_FIG8:
			mesh = Assets::MESH_WORLD_FIG8;
			path = "./assets/worlds/track1.dat";
			break;
		case WORLD_ROUND:
			mesh = Assets::MESH_WORLD_ROUND;
			path = "./assets/worlds/track2.dat";
			break;
		default:
			break;
	}

	if(m_root == nullptr)
		m_root = m_context.smgr->addEmptySceneNode();

	if(m_mesh != nullptr)
		m_mesh->drop();

	m_mesh = m_context.smgr->addAnimatedMeshSceneNode(m_context.assets->getMesh(mesh));
	if(m_mesh->getTriangleSelector() != nullptr)
		m_mesh->getTriangleSelector()->drop();
	m_mesh->setTriangleSelector(m_context.smgr->createOctreeTriangleSelector(m_mesh->getMesh(), m_mesh, 128));
	//m_mesh->addShadowVolumeSceneNode();
	//m_mesh->setDebugDataVisible(scene::EDS_FULL);

	// open and interpret world file
	std::ifstream file(path);
	if(!file.is_open()) {
		throw std::runtime_error("Failed to open world file: " +path);
		return;
	}

	std::string line;
	std::stringstream ss;
	while(std::getline(file, line)) {
		ss = std::stringstream(line);
		char type; ss >> type;
		std::string data;

		switch (type)
		{

		// light scene node
		case 'l': {
			ss >> data; float x = std::stof(data);
			ss >> data; float y = std::stof(data);
			ss >> data; float z = std::stof(data);
			ss >> data; float r = std::stof(data);
			ss >> data; float g = std::stof(data);
			ss >> data; float b = std::stof(data);
			ss >> data; float d = std::stof(data);

			m_nodes.push_back(m_context.smgr->addLightSceneNode(m_mesh, vector3df(x, y, z), video::SColorf(r, g, b, 1), d));
		} break;

		// directional light (e.g. sun, use one)
		case 'd': {
			ss >> data; float x = std::stof(data);
			ss >> data; float y = std::stof(data);
			ss >> data; float z = std::stof(data);
			ss >> data; float r = std::stof(data);
			ss >> data; float g = std::stof(data);
			ss >> data; float b = std::stof(data);
			ss >> data; float d = std::stof(data);

			auto* light = m_context.smgr->addLightSceneNode(m_mesh, vector3df(0, 100, 0), video::SColorf(r, g, b, 1), d);
			light->setLightType(video::ELT_DIRECTIONAL);
			light->setRotation(vector3df(x, y, z));
			light->enableCastShadow(false);
			m_nodes.push_back(light);
		} break;

		// item box
		case 'b': {
			ss >> data; float x = std::stof(data);
			ss >> data; float y = std::stof(data);
			ss >> data; float z = std::stof(data);

			auto* itembox = m_context.smgr->addAnimatedMeshSceneNode(m_context.assets->getMesh(Assets::MESH_ITEMBOX),
				m_mesh, -1, vector3df(x, y, z), {0,0,0}, {2, 2, 2});

			//itembox->addShadowVolumeSceneNode(itembox->getMesh());
			m_itemboxes.push_back(itembox);
		} break;
		
		// checkpoint
		case 'c': {
			ss >> data; float x = std::stof(data);
			ss >> data; float z = std::stof(data);
			ss >> data; float r = std::stof(data);

			m_checkpoints.push_back({{x, 0, z}, r});
		} break;

		default:
			break;
		}
	}

}

scene::IAnimatedMeshSceneNode* World::getMesh() {
	return m_mesh;
}

void World::hide() {
	m_mesh->setVisible(false);
	m_root->setVisible(false);
}

void World::show() {
	m_mesh->setVisible(true);
	m_root->setVisible(true);
}

void World::createItem(Item item, vector3df pos, size_t owner, ActiveItem::ItemData data) {
	scene::IAnimatedMeshSceneNode* mesh = nullptr;
	
	if(item == Item::ITEM_PROJECTILE) {
		mesh = m_context.smgr->addAnimatedMeshSceneNode(m_context.assets->getMesh(Assets::MESH_PROJECTILE), 0, -1, pos, {}, {2,2,2});
	} else if(item == Item::ITEM_SPIKE) {
		mesh = m_context.smgr->addAnimatedMeshSceneNode(m_context.assets->getMesh(Assets::MESH_SPIKE), 0, -1, pos, {}, {1.6,1.6,1.6});
	} else if(item == Item::ITEM_SPIKESHIELD) {
		mesh = m_context.smgr->addAnimatedMeshSceneNode(m_context.assets->getMesh(Assets::MESH_PROJECTILE), 0, -1, pos, {}, {3,3,3});
	}
	
	// the curly struct constructor syntax is so fucking sexy
	m_items.push_back({item, mesh, owner, data}); 

	// lmao i have autism
}

void World::updateItems(float dt, vector3df playerPos) {
	if(dt >= 0.1875) {
		dt = 0.1875;
	}

	while(!m_context.network->getNewItems().empty()) {
		sf::Packet p = m_context.network->getNewItems()[m_context.network->getNewItems().size()-1];
		m_context.network->getNewItems().pop_back();

		// prep new item function call
		sf::Uint32 item;
		vector3df pos;
		size_t owner;
		ActiveItem::ItemData data;

		p >> item >> owner >> pos.X >> pos.Y >> pos.Z >> data.dir.X >> data.dir.Y >> data.dir.Z >> data.dur;
		createItem((World::Item)item, pos, owner, data);
	}

	for(size_t i = 0; i < m_items.size(); i++) {
		auto& item = m_items[i];
		item.data.dur -= dt;

		if(item.type == Item::ITEM_PROJECTILE) {
			auto move = item.mesh->getPosition()+item.data.dir.rotationToDirection()*PROJECTILE_SPEED*dt;
			item.mesh->setPosition(move);
		} else if(item.type == Item::ITEM_SPIKESHIELD) {
			if(item.owner == m_context.network->id())
				item.mesh->setPosition(playerPos);
			else {
				item.mesh->setPosition(m_networksync.getPlayers()[item.owner].second->getAbsolutePosition());
			}
		}

		if(item.data.dur <= 0) {
			item.mesh->remove();
			m_items.erase(m_items.begin()+i);
			i--;
		}
	}
}

void World::reset() {
	hide();
	
	m_checkpoints.clear();
	
	for(auto* node : m_nodes) {
		node->remove();
	}
	
	m_nodes.clear();
	
	for(auto* box : m_itemboxes) {
		box->remove();
	}
	
	m_itemboxes.clear();

	for(auto& item : m_items) {
		item.mesh->remove();
	}

	m_items.clear();
}