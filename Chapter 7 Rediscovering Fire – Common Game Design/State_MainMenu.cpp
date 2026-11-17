#include "State_MainMenu.h"
#include "MemoryManager.h"
#include "StateManager.h"
#include "Window.h"
#include "SharedContext.h"
#include <print>

State_MainMenu::State_MainMenu(StateManager* l_manager)
	: BaseState(l_manager)
	, m_buttonSize{ sf::Vector2f(400.0f, 64.0f) }
	, m_buttonPos{ sf::Vector2f(960.0f, 540.0f) }
	, m_buttonPadding{ 8u }
	, m_font{}
	, m_text{ m_font }
	, m_labels{
		sf::Text{ m_font },
		sf::Text{ m_font },
		sf::Text{ m_font }
	}
{
	// Безопасно загружаем шрифт в поле класса m_font прямо в теле конструктора!
	if (!m_font.openFromFile("C:\\Windows\\Fonts\\Arial.ttf")) [[unlikely]] {
		std::println("[ERROR] Системный шрифт Arial не найден в Windows!");
		exit(EXIT_FAILURE);
	}
}

void State_MainMenu::onCreate() {
	std::pmr::string strForSetString{ "MAIN MENU:", LocalMemoryManager::getManager().getAlloc() };
	m_text.setString(std::string_view(strForSetString));
	m_text.setCharacterSize(20u);

	sf::FloatRect textRect{ m_text.getLocalBounds() };
	m_text.setOrigin({ textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f });
	m_text.setPosition({ 960.0f, 320.0f });

	auto alloc{ LocalMemoryManager::getManager().getAlloc() };
	std::array<std::pmr::string, 3> str{ std::pmr::string("Play", alloc), std::pmr::string("Credits", alloc), std::pmr::string("Exit", alloc) };

	for (size_t i{ 0uz }; i < 3uz; ++i) {
		// Выравнивание кнопок строго по центру экрана
		sf::Vector2f buttonPos{ m_buttonPos.x, 460.0f + (i * (m_buttonSize.y + m_buttonPadding)) };

		m_rects[i].setSize(m_buttonSize);
		m_rects[i].setFillColor(sf::Color::Red);
		m_rects[i].setOrigin({ m_buttonSize.x / 2.0f, m_buttonSize.y / 2.0f });
		m_rects[i].setPosition(buttonPos);

		m_labels[i].setString(std::string_view(str[i]));
		m_labels[i].setCharacterSize(15u);

		sf::FloatRect rect{ m_labels[i].getLocalBounds() };
		m_labels[i].setOrigin({ rect.position.x + rect.size.x / 2.0f, rect.position.y + rect.size.y / 2.0f });
		m_labels[i].setPosition(buttonPos);
	}

	EventManager* eventManager{ m_stateManager->getSharedContext()->m_eventManager };
	eventManager->addCallback(BaseState::StateType::MainMenu, "Mouse_left", &State_MainMenu::mouseClick, this);
}

void State_MainMenu::onDestroy() {
	EventManager* eventManager{ m_stateManager->getSharedContext()->m_eventManager };
	eventManager->removeCallback(BaseState::StateType::MainMenu, "Mouse_left");
}

void State_MainMenu::draw() {
	sf::RenderWindow* window{ m_stateManager->getSharedContext()->m_window->getRenderWindow() };
	window->draw(m_text);

	for (size_t i{ 0uz }; i < 3uz; ++i) {
		window->draw(m_rects[i]);
		window->draw(m_labels[i]);
	}
}

void State_MainMenu::activate() {
	if (m_stateManager->hasState(BaseState::StateType::Game) && m_labels[0].getString() == "Play") [[unlikely]] {
		std::pmr::string strForSetString{ "Resume", LocalMemoryManager::getManager().getAlloc() };
		m_labels[0].setString(std::string_view(strForSetString));
		sf::FloatRect rect{ m_labels[0].getLocalBounds() };
		m_labels[0].setOrigin({ rect.position.x + rect.size.x / 2.0f, rect.position.y + rect.size.y / 2.0f });
	}
}

void State_MainMenu::deactivate() {}
void State_MainMenu::update(const sf::Time l_time) {}

void State_MainMenu::mouseClick(EventDetails* l_details) {
	if (!l_details) return;

	// Читаем живые координаты мыши относительно окна в пикселях и конвертируем
	// в координаты окна (учитывая view / DPI / масштабирование).
	sf::RenderWindow* renderWindow = m_stateManager->getSharedContext()->m_window->getRenderWindow();
	sf::Vector2i pixelPos = m_stateManager->getSharedContext()->m_window->getEventManager()->getMousePosition(renderWindow);
	sf::Vector2f mousePosF = renderWindow->mapPixelToCoords(pixelPos);

	float halfX{ m_buttonSize.x / 2.0f };
	float halfY{ m_buttonSize.y / 2.0f };

	for (size_t i{ 0 }; i < 3uz; ++i) {
		if (mousePosF.x >= m_rects[i].getPosition().x - halfX && mousePosF.x <= m_rects[i].getPosition().x + halfX &&
			mousePosF.y >= m_rects[i].getPosition().y - halfY && mousePosF.y <= m_rects[i].getPosition().y + halfY) [[likely]]
		{
			if (i == 0uz) [[likely]] {
				m_stateManager->switchTo(BaseState::StateType::Game);
			}
			else if (i == 1uz) [[unlikely]] {
				m_stateManager->switchTo(BaseState::StateType::Credits);
			}
			else if (i == 2uz) [[likely]] {
				m_stateManager->getSharedContext()->m_window->close();
			}
			return;
		}
	}
}