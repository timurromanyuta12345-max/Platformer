#pragma once
#include "Window.h"
#include "StateManager.h"
#include "SharedContext.h"
#include "EntityManager.h"
#include "TextureManager.h"

constexpr float timeStepSeconds{ 1.0f / 60.0f };
constexpr sf::Time timeStep{ sf::seconds(timeStepSeconds) };

class Game {
private:
	::Window m_window;
	sf::Clock m_clock;
	sf::Time m_elapsed;
	SharedContext m_sharedContext;
	TextureManager m_textureManager;
	EntityManager m_entityManager;
	StateManager m_stateManager;
	inline static uint32_t frameCount;
	inline static sf::Clock fpsClock;
	//void moveMushroom();
public:
	Game();
	~Game() = default;

	//void handleInput();
	void update();
	void lateUpdate();
	void render();
	void restartClock();

	::Window* getWindow();

	sf::Time getElapsed();
};