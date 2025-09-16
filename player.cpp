#include "player.hpp"
#include "interface.hpp"

Player::Player(Context& context, World& world, NetworkSync& networksync) : m_context(context), m_world(world), m_networksync(networksync) {
	srand(time(0));
}

Player::~Player() {
	if(m_mesh != nullptr)
		m_mesh->drop();
}

bool Player::setup() {

	m_root = m_context.smgr->addEmptySceneNode();

	m_mesh = m_context.smgr->addAnimatedMeshSceneNode(m_context.assets->getMesh(Assets::MESH_CAR_PIXEL), m_root);
	m_meshEnum = Assets::MESH_CAR_PIXEL;
	//m_shadow = m_mesh->addShadowVolumeSceneNode();

	m_texture = m_context.assets->getTexture(Assets::TEXTURE_CARWARM);
	m_texEnum = Assets::TEXTURE_CARWARM;

	m_mesh->setMaterialTexture(0, m_texture);

	//m_mesh->setDebugDataVisible(scene::EDS_FULL);
	
	// add animation collision
	//const core::aabbox3d<f32>& box = m_mesh->getBoundingBox(); core::vector3df radius = box.MaxEdge - box.getCenter(); radius *= 0.55f;
	//m_collision = m_context.smgr->createCollisionResponseAnimator(m_world.getMesh()->getTriangleSelector(), m_root, radius, vector3df(0.0f, 0.0f, 0.0f), vector3df(0.0f, 0.0f, 0.0f));
	//m_root->addAnimator(m_collision);
	m_light = m_context.smgr->addLightSceneNode(0, vector3df(2.0f, 10.f, -3.f), video::SColorf(1.0f, 1.0f, 1.0f, 1.0f), 12.5f);
	m_light->setVisible(false);

	return true;
}

void Player::update(float dt) {

	if(dt >= 0.1875) {
		dt = 0.1875;
	}

	if(m_state == STATE_SELECT) {
		m_root->setPosition(vector3df(0, 5.0f, 0));
		m_root->setRotation(vector3df(0, m_context.timer.getElapsedTime().asSeconds()*100, 0));
		m_context.camera->setPosition(vector3df(0, 8, -8));
		m_context.camera->setTarget(vector3df(0, 3.0f, 0));
		m_camerapos = m_context.camera->getPosition();

		if(!m_light->isVisible()) {
			m_light->setVisible(true);
		}

	} else if(m_state == STATE_PLAYING) {

		if(m_light->isVisible()) {
			m_light->setVisible(false);
		}

		if(!m_context.events->getKeyPressed(KEY_UP)) {
			m_nitro += dt/25.f; // nitro refil
			if(m_nitro > 1) {
				m_nitro = 1;
			}
		}

		// unset whenever collision check should be ignored
		bool collision = true;

		// gravity and drag
		m_velocity.Y -= GRAVITY*dt;
		m_velocity -= m_velocity*(DRAG*dt);

		// drift
		bool drift = m_context.events->getKeyPressed(KEY_DOWN);	
		bool turnleft = m_context.events->getKeyPressed(KEY_KEY_A);
		bool tunrright = m_context.events->getKeyPressed(KEY_KEY_D);

		if(drift && !m_drifting && !m_drifted) {
			if(turnleft || tunrright) {
				if(turnleft) m_driftingdir = -1;
				else m_driftingdir = 1;
				m_drifting = true;
				m_driftclock.restart();
			}
		}

		// i have an uncurable mental illness
		else if(m_drifting && !drift) {
			m_drifting = false;
			sf::Time t = m_driftclock.restart();
			float val = DRIFT_TURBO_SPEED * (t.asSeconds()/5.0f);
			if(val > DRIFT_TURBO_SPEED) val = DRIFT_TURBO_SPEED;
			m_velocity += m_root->getRotation().rotationToDirection()*val;
		} else if(m_drifting && (!turnleft && !tunrright) && !m_driftingcancel) {
			m_driftcancelclock.restart();
			m_driftingcancel = true;
		} else if(m_driftingcancel && (turnleft || tunrright)) {
			m_driftingcancel = false;
		} else if(m_driftingcancel && m_driftcancelclock.getElapsedTime().asSeconds() > 0.5f) {
			m_driftingcancel = false;
			m_drifting = false;
			m_drifted = true;
			m_driftingdir = 0;
		} else if(!m_drifting && drift) {
			m_driftingcancel = false;
			m_drifting = false;
			m_driftingdir = 0;
		}

		if(m_drifted && !drift) {
			m_drifted = false;
		}

		// player movement
		float offsetspeed = 1;
		if(drift) offsetspeed -= 0.145f;
		bool nitroactive = false;
		if(m_context.events->getKeyPressed(KEY_UP) && m_nitro > 0 && m_gamecountdown.getElapsedTime().asSeconds() > 5) { // nitro
			offsetspeed += .7f;
			m_nitro -= dt/3.75f;
			if(m_nitro < 0) {
				m_nitro = 0;
			}
			nitroactive = true;
		}

		if(!raceComplete()) {
			if(m_context.events->getKeyPressed(KEY_KEY_W) || nitroactive) {
				m_velocity += m_root->getRotation().rotationToDirection()*FORWARD_SPEED*offsetspeed*dt;
			} if(m_context.events->getKeyPressed(KEY_KEY_S)) {
				m_velocity -= m_root->getRotation().rotationToDirection()*FORWARD_SPEED*offsetspeed*dt;
			} 
		}

		//if(m_context.events->getKeyPressed(KEY_KEY_R)) {
		//	m_root->setPosition(vector3df(0.0f, 3.0f, 0.0f));
		//	m_velocity = vector3df(0.0f, 0.0f, 0.0f);
		//	collision = false;
		//}


		if(turnleft) {
			m_rotationalvelocity.Y -= ROTATIONAL_SPEED*dt;
		} if(tunrright) {
			m_rotationalvelocity.Y += ROTATIONAL_SPEED*dt;
		}

		// rotate due to drift
		if(m_drifting) {
			m_rotationalvelocity.Y += DRIFT_ROTATIONAL_SPEED*m_driftingdir*dt;
		}


		if(turnleft || tunrright || m_drifting) {
			if(m_rotationalvelocity.Y < -(int)ROTATIONAL_SPEED_CAP * (drift ? ROTATIONAL_SPEED_DRIFT : 1))
				m_rotationalvelocity.Y = -ROTATIONAL_SPEED_CAP * (drift ? ROTATIONAL_SPEED_DRIFT : 1);
			if(m_rotationalvelocity.Y > ROTATIONAL_SPEED_CAP * (drift ? ROTATIONAL_SPEED_DRIFT : 1))
				m_rotationalvelocity.Y = ROTATIONAL_SPEED_CAP * (drift ? ROTATIONAL_SPEED_DRIFT : 1);

			if(m_drifting) {
				if(m_driftingdir == 1) {
					if(m_rotationalvelocity.Y < 0.0f) {
						m_rotationalvelocity.Y = 0.0f;
					}
				}
				else if(m_driftingdir == -1) {
					if(m_rotationalvelocity.Y > 0.0f) {
						m_rotationalvelocity.Y = 0.0f;
					}
				}
			}
		}


		if((turnleft && m_rotationalvelocity.Y > 0.0f) || (tunrright && m_rotationalvelocity.Y < 0.0f) || (!turnleft && !tunrright)) {
			m_rotationalvelocity.Y -= m_rotationalvelocity.Y*ROTATIONAL_DRAG*dt;
		}


		// cast raycasts below car at each corner to hover over the ground
		line3df ray;

		bool hit = false;
		float dist = 0.0f;
		int count = 0;

		// for each corner
		for(int x = -1; x < 2; x += 2) {
			for(int z = -1; z < 2; z += 2) {
				// cast ray with its start based on rotation to direction 
				vector3df rot90 = m_root->getRotation(); rot90.Y += 90.0f;
				ray.start = m_root->getPosition() + (m_root->getRotation().rotationToDirection() * (z*Z_RAY_OFFSET)) + (rot90.rotationToDirection() * (x*X_RAY_OFFSET));
				ray.end = ray.start + vector3df(0.0f, -HOVER_HEIGHT, 0.0f);

				vector3df point; triangle3df tri; scene::ISceneNode* node = nullptr;
				node = m_context.smgr->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(ray, point, tri);

				// if raycast hit
				if(node) {
					hit = true;
					dist += abs(ray.start.Y - point.Y);
					count++;
					if(count > 2) m_lastsafepos = m_root->getPosition();
				}
			}
		}

		if(hit) {
			m_flying = false;
			if(abs(m_flyrotation) > 30.f && m_flyrotation != INFINITY) {
				int rotations = 0;
				while(round(m_flyrotation/90)*90 >= 360) {
					m_flyrotation -= 360;
					rotations++;
				} while(round(m_flyrotation/90)*90 <= -360) {
					m_flyrotation += 360;
					rotations++;
				}

				if(m_flyrotation < 45 && m_flyrotation > -45) {
					// speed boost
					float boost = (DRIFT_TURBO_SPEED*rotations)*0.75f;
					if(boost > DRIFT_TURBO_SPEED*2) {
						boost = DRIFT_TURBO_SPEED*2;
					}
					m_velocity += m_root->getRotation().rotationToDirection()*boost;
					m_rotationalvelocitymesh.Z = 0.0f;
				} else {
					// speed deficeit
					m_velocity = m_velocity/4.f;
				}
			}
			m_flyrotation = INFINITY;

			dist /= count;
			dist = HOVER_HEIGHT - dist;	

			// lift car up
			if(m_velocity.Y < 0.0f) {
				m_velocity.Y += CANCEL_GRAV_FORCE*dt;
			}
			float force = (dist)*HOVER_FORCE*dt;
			if(force > HOVER_FORCE_MAX*dt) force = HOVER_FORCE_MAX*dt;
			m_velocity.Y += force;

		} else {
			m_flying = true;

			if(m_flyrotation == INFINITY) {
				m_flyrotation = m_mesh->getRotation().Z;
			}
		}


		// do a barrel roll!
		bool barrelleft = m_context.events->getKeyPressed(KEY_RIGHT) && !hit;
		bool barrelright = m_context.events->getKeyPressed(KEY_LEFT) && !hit;

		if(barrelleft) {
			m_rotationalvelocitymesh.Z -= ROTATIONAL_SPEED*1.95*dt;
			if(m_rotationalvelocitymesh.Z < -(int)ROTATIONAL_SPEED_ROLL_CAP) m_rotationalvelocitymesh.Z = -ROTATIONAL_SPEED_ROLL_CAP;
		} if(barrelright) {
			m_rotationalvelocitymesh.Z += ROTATIONAL_SPEED*1.95*dt;
			if(m_rotationalvelocitymesh.Z > (int)ROTATIONAL_SPEED_ROLL_CAP) m_rotationalvelocitymesh.Z = ROTATIONAL_SPEED_ROLL_CAP;
		}

		if((!barrelleft && !barrelright)) {
			m_rotationalvelocitymesh.Z -= m_rotationalvelocitymesh.Z*ROTATIONAL_DRAG*4.5*dt;
			if((!barrelleft && !barrelright)) {
				int val = (int)m_mesh->getRotation().Z%360;
				bool a = false, b = false;

				if(val < -180.f) a = true;
				else if(val > 180.f) b = true;
				else if(val < 0.f) b = true;
				else if(val > 0.f) a = true;

				if(a) m_rotationalvelocitymesh.Z -= (ROTATIONAL_SPEED*1.85)*dt;
				if(b) m_rotationalvelocitymesh.Z += (ROTATIONAL_SPEED*1.85)*dt;
				if(val != (int)m_mesh->getRotation().Z) {
					vector3df rot = m_mesh->getRotation(); rot.Z = (int)rot.Z%360;
					m_mesh->setRotation(rot);
				}
			}
		}

		// manipulate camera
		m_context.camera->setTarget(m_root->getPosition() + vector3df(0.0f, CAMERA_OFFSET_VERT, 0.0f));
		vector3df campos = m_root->getPosition() + (m_root->getRotation().rotationToDirection() * -CAMERA_OFFSET_BACK) + vector3df(0.0f, CAMERA_OFFSET_VERT*2.f, 0.0f);

		// interpolate to camera position with dt in mind
		m_camerapos = (m_context.camera->getPosition() + (campos - m_context.camera->getPosition())*dt*10.0f);
		vector3df offset; //= vector3df(Random::getf(-CAMERA_SHAKE, CAMERA_SHAKE), Random::getf(-CAMERA_SHAKE, CAMERA_SHAKE), Random::getf(-CAMERA_SHAKE, CAMERA_SHAKE)) * (m_velocity.getLength()/(FORWARD_SPEED*1.25f));
		m_context.camera->setPosition(m_camerapos + offset);


		// apply transformations to node
		vector3df pos = m_root->getPosition();
		if(m_gamecountdown.getElapsedTime().asSeconds() > 5.f) {
			m_root->setPosition(m_root->getPosition() + m_velocity*dt);
		} else {
			m_velocity = {0,0,0};
		}
		m_root->setRotation(m_root->getRotation() + m_rotationalvelocity*dt);
		m_mesh->setRotation(m_mesh->getRotation() + m_rotationalvelocitymesh*dt);

		if(m_flying) {
			m_flyrotation += m_rotationalvelocitymesh.Z*dt;
		}

		// if player fell off 
		if(m_root->getPosition().Y < -25) {
			m_root->setPosition(m_lastsafepos);
			m_velocity = {0,0,0};
			m_rotationalvelocity = {0,0,0};
			collision = false;
		}

		// final collision check to avoid boundry breaks
		if(collision) {
			ray.start = pos;
			ray.end = m_root->getPosition();
			vector3df point; triangle3df tri; scene::ISceneNode* node = nullptr;
			node = m_context.smgr->getSceneCollisionManager()->getSceneNodeAndCollisionPointFromRay(ray, point, tri);

			// if raycast hit, player has passed through mesh
			if(node) {
				// check if tri is under player
				if(tri.getNormal().normalize().Y > 0.5f) {
					if(m_velocity.Y < 0.0f) {
						m_velocity.Y = -m_velocity.Y*0.65;
					} else {
						m_velocity.Y += HOVER_FORCE*2.5f*(1/tri.getNormal().normalize().Y)*dt;
					}
					if(abs(pos.Y - point.Y) < 0.1f) {
						pos.Y = point.Y + 0.1f;
					}
				} else {
					m_velocity = -m_velocity*0.35f;
				}
				m_root->setPosition(pos);
			}
		}

		// checkpoints
		vector3df playerpos = m_root->getPosition();
		vector3df poscheck = m_checkpoints->at(m_checkpoint).first;
		float checkpointdist = m_checkpoints->at(m_checkpoint).second;
		if(playerpos.X > poscheck.X - checkpointdist && playerpos.X < poscheck.X + checkpointdist) {
			if(playerpos.Z > poscheck.Z - checkpointdist && playerpos.Z < poscheck.Z + checkpointdist) {
				// colliding w checkpoint
				m_checkpoint++;
				if(m_checkpoints->size() == m_checkpoint) {
					m_laps++;

					for(auto& box : m_world.getItemBoxes()) {
						box->setVisible(true);
					}

					m_checkpoint = 0;
					if(m_laps == TOTAL_LAPS) {
						sf::Packet p; p << VICTORY;
						m_context.network->sendTcp(p);
					}
				}
			}
		}
	}

	// check for collision with item boxes
	for(auto& box : m_world.getItemBoxes()) {
		if(box->getTransformedBoundingBox().intersectsWithBox(m_mesh->getTransformedBoundingBox()) && box->isVisible()) {
			box->setVisible(false);

			// who says c style casts are bad 
			m_item = (World::Item)(rand()%((int)World::Item::ITEM_COUNT));
		}
	}
	
	// space to shoot item
	if(m_context.events->getKeyPressed(KEY_SPACE) && hasItem()) {
		
		World::ActiveItem::ItemData data;
		if(m_item == World::Item::ITEM_PROJECTILE) {
			data.dir = m_root->getRotation();
		}

		else if(m_item == World::Item::ITEM_SPIKE) {
			data.dur = 20;
		}

		// server shit againnnnnnn

		sf::Packet itempack;
		auto pos = m_root->getPosition();

		// send base info
		itempack << ITEM << (sf::Uint32)m_item << pos.X << pos.Y << pos.Z;
		itempack << data.dir.X << data.dir.Y << data.dir.Z << data.dur;

		m_context.network->sendTcp(itempack);

		m_item = World::Item::ITEM_COUNT;
	}

	// check for collisions with items
	for(size_t i = 0; i < m_world.getItems().size(); i++) {
		auto& item = m_world.getItems()[i];

		if(item.owner == m_context.network->id()) {
			continue;
		}

		if(item.type == World::ITEM_PROJECTILE || item.type == World::ITEM_SPIKE || item.type == World::ITEM_SPIKESHIELD) {
			if(item.mesh->getTransformedBoundingBox().intersectsWithBox(m_mesh->getTransformedBoundingBox())) {
				m_velocity = {0,20,0};
				m_rotationalvelocitymesh.Z = (rand()%600) - 300;
			}
		}
		
	}

	if(m_context.network->connected()) {
		Data data;
		data.id = m_context.network->id();
		sf::Vector3f t;
		t.x = m_root->getPosition().X; t.y = m_root->getPosition().Y; t.z = m_root->getPosition().Z; data.pos = t;
		t.x = m_velocity.X; t.y = m_velocity.Y; t.z = m_velocity.Z; data.vel = t;
		t.x = m_root->getRotation().X; t.y = m_root->getRotation().Y; t.z = m_root->getRotation().Z; data.rot = t;
		t.x = m_mesh->getRotation().X; t.y = m_mesh->getRotation().Y; t.z = m_mesh->getRotation().Z; data.mrot = t;
		m_context.network->updateData(data);
	}
	
}

void Player::setState(State state) {
	m_state = state;
	if(m_state == STATE_PLAYING) {
		m_gamecountdown.restart();
		m_root->setRotation({0,0,0});
		m_checkpoints = &m_world.checkpoints();
	}
}

float Player::speed() {
	return sqrt(m_velocity.X*m_velocity.X + m_velocity.Z*m_velocity.Z);
}

float Player::nitro() {
	return m_nitro;
}

void Player::selectCar(std::pair<std::bitset<4>, std::bitset<4>> bits) {
	scene::IAnimatedMesh* mesh = nullptr;
	if(bits.first.any()){
		for(size_t i = 0; i < 4; i++) {
			if(bits.first[i]) {
				m_meshEnum = (Assets::Mesh)((int)Assets::MESH_CAR_PIXEL+i);
				mesh = m_context.assets->getMesh(m_meshEnum);
			}
		}
	} else {
		for(size_t i = 0; i < 4; i++) {
			if(bits.second[i]) {
				m_meshEnum = (Assets::Mesh)((int)Assets::MESH_CAR_PIXEL+4+i);
				mesh = m_context.assets->getMesh(m_meshEnum);
			}
		}
	}

	if(mesh != nullptr) {
		m_mesh->setMesh(mesh);
		//if(m_shadow)
		//	m_shadow->remove();
		//if(m_meshEnum != Assets::MESH_CAR_RING)
		//	m_shadow = m_mesh->addShadowVolumeSceneNode();
		//else m_shadow = nullptr;
		m_mesh->setMaterialTexture(0, m_texture);
	}
}

void Player::selectTexture(std::pair<std::bitset<4>, std::bitset<4>> bits) {
	video::ITexture* tex = nullptr;
	if(bits.first.any()){
		for(size_t i = 0; i < 4; i++) {
			if(bits.first[i]) {
				m_texEnum = (Assets::Texture)((int)Assets::TEXTURE_CARWARM+i);
				tex = m_context.assets->getTexture(m_texEnum);
			}
		}
	} else {
		for(size_t i = 0; i < 4; i++) {
			if(bits.second[i]) {
				m_texEnum = (Assets::Texture)((int)Assets::TEXTURE_CARWARM+4+i);
				tex = m_context.assets->getTexture(m_texEnum);
			}
		}
	}

	if(tex != nullptr) {
		m_texture = tex;
		m_mesh->setMaterialTexture(0, m_texture);
	}
}

size_t Player::checkpoint() {
	return m_checkpoint;
}

size_t Player::laps() {
	return m_laps;
}