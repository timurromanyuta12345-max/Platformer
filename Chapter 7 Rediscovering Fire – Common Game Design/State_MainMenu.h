#pragma once
#include "BaseState.h"
#include <array>

struct EventDetails;

class State_MainMenu : public BaseState {
private:
	sf::Vector2f m_buttonSize;
	sf::Vector2f m_buttonPos;
	uint32_t m_buttonPadding;
	sf::Font m_font;
	sf::Text m_text;
	std::array<sf::RectangleShape, 3> m_rects;
	std::array<sf::Text, 3> m_labels;

public:
	State_MainMenu(StateManager* l_manager);
	virtual ~State_MainMenu() noexcept override = default;

	void mouseClick(EventDetails* l_details);

	virtual void onCreate() override;
	virtual void onDestroy() override;
	virtual void draw() override;
	virtual void activate() override;
	virtual void deactivate() override;
	virtual void update(const sf::Time l_time) override;
};