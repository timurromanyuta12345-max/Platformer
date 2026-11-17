#pragma once
#include "BaseState.h"

struct EventDetails;

class State_Paused : public BaseState {
private:
	sf::Font m_font;
	sf::Text m_text;
	sf::RectangleShape m_rect;
public:
	State_Paused(StateManager* l_manager);

	virtual void onCreate() override;
	virtual void onDestroy() override;
	virtual void draw() override;
	virtual void activate() override;
	virtual void deactivate() override;
	virtual void update(const sf::Time l_time) override;

	void unpause(EventDetails* l_details);
};