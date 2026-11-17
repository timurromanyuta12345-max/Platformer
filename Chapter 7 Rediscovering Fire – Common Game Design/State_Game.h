#pragma once
#include "BaseState.h"

struct EventDetails;
class Map;

class State_Game : public BaseState {
private:
	/*sf::Texture m_texture;
	sf::Sprite m_sprite;
	sf::Vector2f m_increment;*/
	Map* m_gameMap;
public:
	State_Game(StateManager* l_manager);

	virtual void onCreate() override;
	virtual void onDestroy() override;
	virtual void draw() override;
	virtual void activate() override;
	virtual void deactivate() override;
	virtual void update(const sf::Time l_time) override;

	void mainMenu(EventDetails* l_details);
	void pause(EventDetails* l_details);
	//void moveSprite(EventDetails* l_details);
};