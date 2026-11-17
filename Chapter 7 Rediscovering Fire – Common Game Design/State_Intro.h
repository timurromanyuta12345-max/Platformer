#pragma once
#include "BaseState.h"

struct EventDetails;
class SpriteSheet;

class State_Intro : public BaseState {
private:
	/*sf::Texture m_introTexture;
	sf::Sprite m_introSprite;*/
	SpriteSheet* m_spriteSheet;
	sf::Font m_font;
	sf::Text m_text;
	float m_timePassed;

public:
	State_Intro(StateManager* l_stateManager);
	virtual ~State_Intro() noexcept override = default;

	void continueCustom(EventDetails* l_details);

	virtual void onCreate() override;
	virtual void onDestroy() override;
	virtual void draw() override;
	virtual void activate() override;
	virtual void deactivate() override;
	virtual void update(const sf::Time l_time) override;
};
