#include "event.hpp"

bool EventHandler::OnEvent(const SEvent& e)
{
	if(e.EventType == EEVENT_TYPE::EET_KEY_INPUT_EVENT)
	{
		m_keys[e.KeyInput.Key] = e.KeyInput.PressedDown;

		if(!e.KeyInput.PressedDown)
			m_keys_prev[e.KeyInput.Key] = false;
	}

	// pass event to imgui
	event_to_irrimgui(e);

	return false;
}

bool EventHandler::getKeyPressed(EKEY_CODE k) const
{
	return m_keys[k];
}

bool EventHandler::getKeyReleased(EKEY_CODE k)
{
	if(m_keys[k] != m_keys_prev[k] && m_keys[k] == true) {
		m_keys_prev[k] = true;
		return true;
	}
	else return false;
}

void EventHandler::reset()
{
	for (size_t i = 0; i < KEY_KEY_CODES_COUNT; i++)
		m_keys[i] = false;
}