#include "Game.h"

Game::Game()
	: m_window{ "Chapter 7", sf::Vector2u(1920u, 1080u) }
	, m_entityManager{&m_sharedContext, 2u}
	, m_stateManager{&m_sharedContext}
{
	m_sharedContext.m_window = &m_window;
	m_sharedContext.m_eventManager = m_window.getEventManager();
	m_sharedContext.m_textureManager = &m_textureManager;
	m_sharedContext.m_entityManager = &m_entityManager;

	m_stateManager.switchTo(BaseState::StateType::Intro); // Use default constructor
}

void Game::update() {
	m_window.update();
	frameCount++;

	if (fpsClock.getElapsedTime() >= sf::seconds(1.0f)) [[unlikely]] {
		std::pmr::string windowStr((std::format("Chapter 7 | FPS: {}", frameCount)), LocalMemoryManager::getManager().getAlloc());
		m_window.getWindow().setTitle(std::string_view(windowStr));
		frameCount = 0u;
		fpsClock.restart();
	}

	// Spiral of Death
	if (m_elapsed > sf::Time(sf::seconds(0.25f))) {
		m_elapsed = sf::Time(sf::seconds(0.25f));
	}

	while (m_elapsed >= timeStep) {
		m_window.updateInput();
		m_stateManager.update(timeStep); // Use default constructor
		m_elapsed -= timeStep;
	}
}

void Game::render() {
	m_window.beginDraw();
	m_stateManager.draw(); // Use default constructor
	m_window.endDraw();
}

/*void Game::handleInput() {
	m_window.update();
}*/

void Game::restartClock() {
	m_elapsed += m_clock.restart();
}

::Window* Game::getWindow() {
	return &m_window;
}

sf::Time Game::getElapsed() {
	return m_elapsed;
}

void Game::lateUpdate() {
	m_stateManager.processRequests(); // Use default constructor
	restartClock();
}