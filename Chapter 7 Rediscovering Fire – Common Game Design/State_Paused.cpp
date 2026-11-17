#include "State_Paused.h"
#include "StateManager.h"
#include "MemoryManager.h"
#include "Window.h"
#include "SharedContext.h"
#include <print>

State_Paused::State_Paused(StateManager* l_manager) : BaseState(l_manager), m_font{}, m_text{ m_font } {
	// Безопасно загружаем шрифт в поле класса m_font прямо в теле конструктора!
	if (!m_font.openFromFile("C:\\Windows\\Fonts\\Arial.ttf")) [[unlikely]] {
		std::println("[ERROR] Системный шрифт Arial не найден в Windows!");
		exit(EXIT_FAILURE);
	}
}

void State_Paused::onCreate() {
	setTransparent(true);

	std::pmr::string strForSetString{ "PAUSED", LocalMemoryManager::getManager().getAlloc() };
	m_text.setString(std::string_view(strForSetString));
	m_text.setCharacterSize(15u);
	m_text.setStyle(sf::Text::Bold);

	sf::Vector2u windowSize{ m_stateManager->getSharedContext()->m_window->getWindowSize() };
	sf::FloatRect textRect{ m_text.getLocalBounds() };
	m_text.setOrigin({ textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f });
	m_text.setPosition({ windowSize.x / 2.0f, windowSize.y / 2.0f });

	m_rect.setSize(sf::Vector2f(windowSize));
	m_rect.setPosition({ 0.0f, 0.0f });
	m_rect.setFillColor(sf::Color(0, 0, 0, 150));

	EventManager* eventManager{ m_stateManager->getSharedContext()->m_eventManager };
	eventManager->addCallback(BaseState::StateType::Paused, "Key_p", &State_Paused::unpause, this);
}

void State_Paused::onDestroy() {
	EventManager* eventManager{ m_stateManager->getSharedContext()->m_eventManager };
	eventManager->removeCallback(BaseState::StateType::Paused, "Key_p");
}

void State_Paused::draw() {
	sf::RenderWindow* window{ m_stateManager->getSharedContext()->m_window->getRenderWindow() };
	window->draw(m_rect);
	window->draw(m_text);
}

void State_Paused::unpause(EventDetails* l_details) {
	m_stateManager->switchTo(BaseState::StateType::Game);
}

void State_Paused::activate() {}
void State_Paused::deactivate() {}
void State_Paused::update(const sf::Time l_time) {}