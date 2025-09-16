#include "interface.hpp"

Interface::Interface(Context& context, Player& player, NetworkSync& networksync) : m_context(context), m_player(player), m_networksync(networksync) {

}

Interface::~Interface() {

}

void Interface::setup() {

	// setup imgui binding
	IrrIMGUI::SIMGUISettings settings;
	settings.mIsGUIMouseCursorEnabled = false;
	settings.mIsIMGUIMemoryAllocationTrackingEnabled = false;

	IrrIMGUI::IIMGUIHandle *imgui = IrrIMGUI::createIMGUI(m_context.device, m_context.events, &settings);
	m_context.imgui = imgui;

	ImGui::GetIO().IniFilename = nullptr;

	// load fonts
	m_fonttitle = imgui->addFontFromFileTTF("assets/fonts/title.ttf", 64.0f);
	m_fonttext = imgui->addFontFromFileTTF("assets/fonts/text.ttf", 32.0f);
	if(!m_fonttitle) throw std::runtime_error("Could not load title font");

	imgui->compileFonts();
}

void Interface::render() {
	m_context.imgui->startGUI();

	ImGui::Begin("##MAIN WINDOW", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	auto winsize = m_context.driver->getScreenSize();
	ImGui::SetWindowPos(ImVec2(0,0), ImGuiCond_Once);
	ImGui::SetWindowSize(ImVec2(winsize.Width, winsize.Height));

	switch (m_state)
	{
	case STATE_LOAD:
		ImGui::PushFont(m_fonttitle);
		centerByHeight(ImGui::CalcTextSize("Loading assets...").y);
		centeredText("Loading assets...");
		ImGui::PopFont();
		break;
	
	case STATE_MENU: {
		ImGui::PushFont(m_fonttitle);
		centeredText("Racing Game");
		ImGui::SetCursorPosY(winsize.Height/2);

		std::bitset<2> b1 = centeredButtonRowE<2>({"Play", "Connect"}, ImVec2(400, 100));
		std::bitset<2> b2 = centeredButtonRowE<2>({m_context.network->connected() ? "X" : "Car", "Credits"}, ImVec2(400, 100));

		ImGui::Dummy(ImVec2(0,100));

		ImGui::PushFont(m_fonttext);
		centeredText("Player Name");
		centerByWidth(400);
		ImGui::PushItemWidth(400);
		ImGui::InputText("##name", &m_name, m_context.network->connected() ? ImGuiInputTextFlags_ReadOnly : 0);
		ImGui::PopFont();

		if(b1[0]) {
			m_state = STATE_SELECTT;
		} else if(b1[1]) {
			m_state = STATE_CONNECT;
		}

		if(b2[0]) {
			if(!m_context.network->connected())
				m_state = STATE_SELECTC;
		} else if(b2[1]) {
			m_state = STATE_MENU;
		}

		ImGui::PopFont();

		} break;


	case STATE_SELECTT: {
		ImGui::PushFont(m_fonttitle);
		if(m_context.network->connected()) {
			centeredText("Vote For a Track");
		} else {
			centeredText("Select Track");
		}
		ImGui::SetCursorPosY(winsize.Height/2);

		ImGui::PopFont();
		ImGui::PushFont(m_fonttext);

		// 2x4 grid of buttons
		std::bitset<4> b1 = centeredButtonRowE<4>({"Track 1", "Track 2", "Track 3", "Track 4"}, ImVec2(300, 100));
		std::bitset<4> b2 = centeredButtonRowE<4>({"Track 5", "Track 6", "Track 7", "Track 8"}, ImVec2(300, 100));
		centerByWidth(300);
		if(ImGui::Button("Back", ImVec2(300, 100))) {
			m_state = STATE_MENU;
		}

		sf::Uint32 map = -1;

		if(b1.any()) {
			if(b1[0]) map = 0;
			if(b1[1]) map = 1;
			if(b1[2]) map = 2;
			if(b1[3]) map = 3;
		} else if(b2.any()) {
			if(b2[0]) map = 4;
			if(b2[1]) map = 5;
			if(b2[2]) map = 6;
			if(b2[3]) map = 7;

		}

		if(b1.any() || b2.any()) {
			if(m_context.network->connected()) {
				m_state = STATE_WAIT;
				sf::Packet p; p << VOTE << map;
				m_context.network->sendTcp(p);
			} else {
				m_state = STATE_GAME;
				m_player.getWorld().loadWorld((World::WorldName)map);
				m_player.setState(Player::STATE_PLAYING);
			}
		}


		ImGui::PopFont();
		} break;

	case STATE_SELECTC: {
		ImGui::PushFont(m_fonttitle);
		centeredText("Select Car");
		ImGui::SetCursorPosY(winsize.Height/1.75f);

		ImGui::PopFont();
		ImGui::PushFont(m_fonttext);

		std::bitset<4> m1 = centeredButtonRowE<4>({"Car 1", "Car 2", "Car 3", "Car 4"}, ImVec2(300, 70));
		std::bitset<4> m2 = centeredButtonRowE<4>({"Car 5", "Car 6", "Car 7", "Car 8"}, ImVec2(300, 70));

		ImGui::Dummy(ImVec2(0, 25));

		std::bitset<4> t1 = centeredButtonRowE<4>({"Tex 1", "Tex 2", "Tex 3", "Tex 4"}, ImVec2(300, 70));
		std::bitset<4> t2 = centeredButtonRowE<4>({"Tex 5", "Tex 6", "Tex 7", "Tex 8"}, ImVec2(300, 70));


		centerByWidth(300);
		if(ImGui::Button("Done", ImVec2(300, 100))) {
			m_state = STATE_MENU;
		}

		m_player.selectCar({m1, m2});
		m_player.selectTexture({t1, t2});

		ImGui::PopFont();
		} break;

	case STATE_CONNECT: {
		ImGui::PushFont(m_fonttitle);
		centeredText("Connect to server");
		ImGui::SetCursorPosY(winsize.Height/2);

		ImGui::PopFont();
		ImGui::PushFont(m_fonttext);

		if(!m_context.network->connected()) {
			static std::string ip = "192.168.1.160";
			static int port = 3417;

			centerByWidth(700);
			ImGui::PushItemWidth(700);
			ImGui::InputText("IP", &ip, 16);

			centerByWidth(700);
			ImGui::PushItemWidth(700);
			ImGui::InputInt("Port", &port);

			ImGui::SetCursorPosY(winsize.Height-400);
			if(!m_context.network->connecting()) {
				std::bitset<2> b = centeredButtonRowE<2>({"Connect", "Back"}, ImVec2(200, 100));
				if(b[0]) {
					m_context.network->setup(ip, port, m_name, m_player.getMeshEnum(), m_player.getTexEnum());
				} else if(b[1]) {
					m_state = STATE_MENU;
				}
			} else {
				ImGui::SetCursorPosY(winsize.Height-400);
				centeredText("Connecting...");
			}
		} else {
			ImGui::SetCursorPosY(winsize.Height-400);
			centeredText("Connected");
			centerByWidth(300);
			if(ImGui::Button("Back", ImVec2(300, 100))) {
				m_state = STATE_MENU;
			}
		}
		
		ImGui::PopFont();
		} break;

	case STATE_WAIT: {
		ImGui::PushFont(m_fonttitle);
		centeredText("Waiting for server...");
		ImGui::SetCursorPosY(winsize.Height-250);
		centerByWidth(300);
		if(ImGui::Button("Start game", ImVec2(300, 100))) {
			sf::Packet p; p << BEGIN;
			m_context.network->sendTcp(p);
		}

		if(m_context.network->gameBegun()) {
			m_state = STATE_GAME;
			m_player.getWorld().loadWorld((World::WorldName)m_context.network->getMap());
			m_player.setState(Player::STATE_PLAYING);
		}

		ImGui::PopFont();
	} break;

	case STATE_GAME: {

		if(m_player.countdown() < 6.f) {
			ImGui::PushFont(m_fonttext);
			ImGui::PushStyleColor(ImGuiCol_Text, {0, 0, 0, 1});
			float val = 5.f - m_player.countdown();
			if(val < 0) val = 0;
			centeredText((std::format("Go in: {:.2f}", val)).c_str()); // finally std::format
			ImGui::PopStyleColor();
			ImGui::PopFont();
		}

		ImGui::PushFont(m_fonttext);
		if(!m_player.raceComplete()) {
			// info
			ImGui::SetNextWindowPos(ImVec2(10, winsize.Height-300));
			if(ImGui::BeginChild("##child", ImVec2(600, 300), false, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs)) {
				ImGui::Text("Speed: %d KM/H", (int)(m_player.speed()*1.875));
				ImGui::ProgressBar(m_player.nitro(), ImVec2(280, 0), "Nitro");
				ImGui::Text("Checkpoint: %llu", m_player.checkpoint());
				ImGui::Text("Lap: %llu", m_player.laps()+1);
			}
			ImGui::EndChild();

			// items
			ImGui::SetNextWindowPos(ImVec2(30, 30));
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15, 0.15, 0.4, 0.7));
			if(ImGui::BeginChild("##child2", ImVec2(100, 100), true, ImGuiWindowFlags_NoInputs)) {
				// if item display it
				if(m_player.hasItem()) {
					centeredText(std::to_string(m_player.item()).c_str());
				}
			}
			ImGui::PopStyleColor();
			ImGui::EndChild();
		}

		// victory 
		if(m_context.network->gameOver()) {
			ImGui::SetCursorPosY(150);
			ImGui::PushFont(m_fonttext);
			ImGui::PushStyleColor(ImGuiCol_Text, {0, 0, 0, 1});
			centeredText(std::format("Race complete! {}st place!", m_context.network->placement()+1).c_str());
			ImGui::PopStyleColor();
			ImGui::PopFont();
		} else if(m_player.raceComplete()) {
			ImGui::SetCursorPosY(150);
			ImGui::PushFont(m_fonttext);
			ImGui::PushStyleColor(ImGuiCol_Text, {0, 0, 0, 1});
			centeredText("Race complete!");
			centeredText("Waiting for others...");
			ImGui::PopStyleColor();
			ImGui::PopFont();
		}

		// fake 3d name rendering
		ImGui::PushFont(m_fonttext);
		auto origin = m_context.camera->getPosition();
		for(auto& car : m_networksync.getPlayers()) {

			// shortest irrlicht function call
			vector2di pos = m_context.device->getSceneManager()->getSceneCollisionManager()->getScreenCoordinatesFrom3DPosition(car.second.second->getAbsolutePosition());


			float scale = 1 - (car.second.second->getAbsolutePosition().getDistanceFrom(origin)/225);
			if(scale < 0) scale = 0;
			else if(scale > 1) scale = 1;

			ImGui::SetCursorPos(ImVec2(pos.X + 30*scale, pos.Y + 30*scale));
			ImGui::SetWindowFontScale(scale);
			ImGui::Text(m_context.network->getClient(car.first).name.c_str());
			ImGui::SetWindowFontScale(1);
		}
		ImGui::PopFont();

		ImGui::PopFont();
		} break;

	default:
		break;
	}

	ImGui::End();

	m_context.imgui->drawAll();

}

void Interface::setState(Interface::State state) {
	m_state = state;
}

void Interface::centerByHeight(const float height) {
	ImGui::SetCursorPosY((ImGui::GetWindowHeight()-height)*0.5f);
}

void Interface::centerByWidth(const float width) {
	ImGui::SetCursorPosX((ImGui::GetWindowWidth()-width)*0.5f);
}

void Interface::centeredText(const char* text) {
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x-ImGui::CalcTextSize(text).x)/2);
	ImGui::Text(text);
}

Interface::State Interface::getState() const {
	return m_state;
}