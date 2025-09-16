#include <irrlicht.h>
#include <IrrIMGUI/IrrIMGUI.h>

#include "etoirr.hpp"

#pragma once

using namespace irr;

class EventHandler : public IEventReceiver, public IrrIMGUI::CIMGUIEventStorage
{
public:
	bool OnEvent(const SEvent& e);
	bool getKeyPressed(EKEY_CODE k) const;
	bool getKeyReleased(EKEY_CODE k);
	void reset();

private:
	bool m_keys[KEY_KEY_CODES_COUNT] {};
	bool m_keys_prev[KEY_KEY_CODES_COUNT] {};
};