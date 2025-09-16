#include <irrlicht.h>
#include <irrIMGUI/IrrIMGUI.h>
#include <misc/cpp/imgui_stdlib.h>
#include <imgui.h>
#include <bitset>
#include <format>

#include "utils.hpp"
#include "player.hpp"

#pragma once

class Interface {
public:
	Interface(Context& context, Player& player, NetworkSync& networksync);
	~Interface();

	enum State {
		STATE_LOAD,
		STATE_MENU,
		STATE_WAIT,
		STATE_GAME,
		STATE_SELECTT,
		STATE_SELECTC,
		STATE_CONNECT,
		STATE_COUNT
	};

	void render();
	void setup();
	void setState(State state);
	State getState() const;

private:
	// utility
	void centerByWidth(const float width);
	void centerByHeight(const float height);
	void centeredText(const char* text);
	template<int T> std::bitset<T> centeredButtonRow(const std::array<const char*, T> titles, const std::array<ImVec2, T> sizes);
	template<int T> std::bitset<T> centeredButtonRowE(const std::array<const char*, T> titles, const ImVec2 size);

	Context& m_context;
	State m_state = STATE_LOAD;
	Player& m_player;
	NetworkSync& m_networksync;

	ImFont* m_fonttitle = nullptr;
	ImFont* m_fonttext = nullptr;

	std::string m_name;
};

// this is the implementation of the template function, it has to be in the header file
template<int T> std::bitset<T> Interface::centeredButtonRow(const std::array<const char*, T> titles, const std::array<ImVec2, T> sizes)
{
	float culm = 0;
	float winwidth = ImGui::GetWindowWidth();
	std::bitset<T> bitset;
	for (size_t i = 0; i < sizes.size(); i++)
		culm += sizes[i].x;
	ImGui::SetCursorPosX((winwidth-(culm+ImGui::GetStyle().ItemSpacing.x*(titles.size()-1)))*0.5f);
	for (size_t i = 0; i < titles.size(); i++)
	{
		bitset[i] = ImGui::Button(titles[i], sizes[i]);
		if(i != titles.size()-1)
			ImGui::SameLine();
	}

	return bitset;
}

// wrapper function for ease of use
template<int T> std::bitset<T> Interface::centeredButtonRowE(const std::array<const char*, T> titles, const ImVec2 size) {
	
	std::array<ImVec2, T> sizes;
	for (size_t i = 0; i < T; i++) // yes, its a hack, but it works
		sizes[i] = size;
	return centeredButtonRow<T>(titles, sizes);
}