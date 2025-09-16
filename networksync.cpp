#include "networksync.hpp"

NetworkSync::NetworkSync(Context& context) : m_context(context) {
}

NetworkSync::~NetworkSync() {
}

void NetworkSync::update(float dt) {
	if(!m_context.network->connected()) return;

	static auto& deque = m_context.network->serverData();

	// check if new connections are available
	auto pair = m_context.network->newClient();
	while(pair.first) {
		std::cout << "new client: " << pair.second.id << ' ' << pair.second.mesh << ' ' << pair.second.tex << ' ' << pair.second.name << std::endl;

		scene::ISceneNode* node = m_context.smgr->addEmptySceneNode();
		scene::IAnimatedMeshSceneNode* subnode = m_context.smgr->addAnimatedMeshSceneNode(m_context.assets->getMesh((Assets::Mesh)pair.second.mesh), node);
		subnode->setMaterialTexture(0, m_context.assets->getTexture((Assets::Texture)pair.second.tex));
		m_players[pair.second.id] = {node, subnode};
		m_playervel[pair.second.id] = {0, 0, 0};
		pair = m_context.network->newClient();
	}

	if(deque.size() > 0 && m_context.network->serverUpdate()) {
		for(auto& data : deque) {
			if(data.id == m_context.network->id()) continue;
			if(m_players.find(data.id) == m_players.end()) continue;

			vector3df vec = {data.pos.x, data.pos.y, data.pos.z};
			m_players[data.id].first->setPosition(vec);

			vec = {data.rot.x, data.rot.y, data.rot.z};
			m_players[data.id].first->setRotation(vec);

			vec = {data.mrot.x, data.mrot.y, data.mrot.z};
			m_players[data.id].second->setRotation(vec);

			vec = {data.vel.x, data.vel.y, data.vel.z};
			m_playervel[data.id] = vec;
		}
	}

	for(auto& pair : m_players) {
		pair.second.first->setPosition(pair.second.first->getPosition() + m_playervel[pair.first] * dt);
	}
	
}