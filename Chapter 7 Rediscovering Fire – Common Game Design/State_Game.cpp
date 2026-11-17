#include "State_Game.h"
#include "StateManager.h"
#include "EventManager.h"
#include "SharedContext.h"
#include "Window.h"
#include "Map.h"
#include "EntityBase.h"
#include "EntityManager.h"
#include <cmath>
#include <print>

State_Game::State_Game(StateManager* l_manager) : BaseState{ l_manager }, m_gameMap{ nullptr } /*m_texture{
	[]() {
	sf::Texture texture;
	if (!texture.loadFromFile("C:\\Users\\admin\\Pictures\\super-mario-red-mushroom-laser-die-cut-50.jpg")) [[unlikely]] {
		exit(EXIT_FAILURE);
	}
	return texture;
}() }, m_sprite{ m_texture }, m_increment{ 200.0f, 200.0f }*/ {}

void State_Game::onCreate() {
	//m_sprite.setPosition({ 0.0f, 0.0f });

	EventManager* eventManager{ m_stateManager->getSharedContext()->m_eventManager };
	eventManager->addCallback(BaseState::StateType::Game, "Key_escape", &State_Game::mainMenu, this);
	eventManager->addCallback(BaseState::StateType::Game, "Key_p", &State_Game::pause, this);
	//eventManager->addCallback(BaseState::StateType::Game, "Move", &State_Game::moveSprite, this);

	sf::Vector2u size{ m_stateManager->getSharedContext()->m_window->getWindowSize() };
	m_view.setSize(sf::Vector2f(size.x, size.y));
	m_view.setCenter(sf::Vector2f(size.x / 2, size.y / 2));
	m_view.zoom(0.625f);
	m_stateManager->getSharedContext()->m_window->getRenderWindow()->setView(m_view);

	void* rawMemory{ LocalMemoryManager::getManager().getAlloc().allocate(sizeof(Map)) };
	m_gameMap = std::construct_at(reinterpret_cast<Map*>(rawMemory), m_stateManager->getSharedContext(), this);
	m_gameMap->loadMap("Media/Maps/Map1.map");
}

void State_Game::onDestroy() {
	EventManager* eventManager{ m_stateManager->getSharedContext()->m_eventManager };
	eventManager->removeCallback(BaseState::StateType::Game, "Key_escape");
	eventManager->removeCallback(BaseState::StateType::Game, "Key_p");
	//eventManager->removeCallback(BaseState::StateType::Game, "Move");

	if (m_gameMap) {
		std::destroy_at(m_gameMap);
		LocalMemoryManager::getManager().deallocate(m_gameMap, sizeof(Map));
		m_gameMap = nullptr;
	}
}

void State_Game::update(const sf::Time l_time) {
	/*sf::Vector2u windowSize{m_stateManager->getSharedContext()->m_window->getWindowSize()};
	sf::Vector2u textureSize{ m_texture.getSize() };

	if ((m_sprite.getPosition().x > windowSize.x - textureSize.x && m_increment.x > 0.0f) || (m_sprite.getPosition().x < 0.0f && m_increment.x < 0.0f)) [[unlikely]] {
		m_increment.x = -m_increment.x;
	}

	if ((m_sprite.getPosition().y > windowSize.y - textureSize.y && m_increment.y > 0.0f) || (m_sprite.getPosition().y < 0.0f && m_increment.y < 0.0f)) [[unlikely]] {
		m_increment.y = -m_increment.y;
	}

	m_sprite.setPosition({ m_sprite.getPosition().x + (m_increment.x * l_time.asSeconds()), m_sprite.getPosition().y + (m_increment.y * l_time.asSeconds()) });*/

	SharedContext* context{ m_stateManager->getSharedContext() };
	EntityBase* player{ context->m_entityManager->find("Player") };

	if (!player) [[unlikely]] {
		std::println("Respawning player...");
		context->m_entityManager->add(EntityType::Player, "Player");
		player = context->m_entityManager->find("Player");
		player->setPosition(static_cast<sf::Vector2f>(m_gameMap->getPlayerStart()));
	}
	else [[likely]] {
		sf::Vector2f viewCenter{ player->getPosition() };
		viewCenter.y -= m_view.getSize().y * 0.4f;
		m_view.setCenter(viewCenter);
		context->m_window->getRenderWindow()->setView(m_view);
	}

	sf::FloatRect viewspace{ context->m_window->getViewSpace() };
	if (viewspace.position.x <= 0.0f) {
		m_view.setCenter(sf::Vector2f(viewspace.size.x / 2.0f, m_view.getCenter().y));
		context->m_window->getRenderWindow()->setView(m_view);
	}
	else if (viewspace.position.x + viewspace.size.x > m_gameMap->getMapSize().x * Sheet::Tile_Size) {
		m_view.setCenter(sf::Vector2f((m_gameMap->getMapSize().x * Sheet::Tile_Size) - (viewspace.size.x / 2.0f), m_view.getCenter().y));
		context->m_window->getRenderWindow()->setView(m_view);
	}
	viewspace = context->m_window->getViewSpace();
	if (viewspace.position.y <= 0.0f) {
		m_view.setCenter(sf::Vector2f(m_view.getCenter().x, viewspace.size.y / 2.0f));
		context->m_window->getRenderWindow()->setView(m_view);
	}
	else if (viewspace.position.y + viewspace.size.y > m_gameMap->getMapSize().y * Sheet::Tile_Size) {
		m_view.setCenter(sf::Vector2f(m_view.getCenter().x, (m_gameMap->getMapSize().y * Sheet::Tile_Size) - (viewspace.size.y / 2.0f)));
		context->m_window->getRenderWindow()->setView(m_view);
	}

	const sf::Vector2u windowSize{ context->m_window->getWindowSize() };
	const sf::Vector2f worldUnitsPerPixel{
		m_view.getSize().x / static_cast<float>(windowSize.x),
		m_view.getSize().y / static_cast<float>(windowSize.y)
	};

	sf::Vector2f snappedCenter{ m_view.getCenter() };
	snappedCenter.x = std::round(snappedCenter.x / worldUnitsPerPixel.x) * worldUnitsPerPixel.x;
	snappedCenter.y = std::round(snappedCenter.y / worldUnitsPerPixel.y) * worldUnitsPerPixel.y;
	m_view.setCenter(snappedCenter);
	context->m_window->getRenderWindow()->setView(m_view);

	m_gameMap->update(l_time.asSeconds());
	m_stateManager->getSharedContext()->m_entityManager->update(l_time.asSeconds());
}

void State_Game::draw() {
	if (m_gameMap) {
		m_gameMap->draw();
	}
	m_stateManager->getSharedContext()->m_entityManager->draw();
}

void State_Game::mainMenu(EventDetails* l_details) {
	m_stateManager->switchTo(BaseState::StateType::MainMenu);
}

void State_Game::pause(EventDetails* l_details) {
	m_stateManager->switchTo(BaseState::StateType::Paused);
}

/*void State_Game::moveSprite(EventDetails* l_details) {
	if (!l_details) return;
	sf::Vector2i mousePosition{ l_details->m_mouse.x, l_details->m_mouse.y };
	m_sprite.setPosition(sf::Vector2f(mousePosition));
}*/

void State_Game::activate() {}
void State_Game::deactivate() {}