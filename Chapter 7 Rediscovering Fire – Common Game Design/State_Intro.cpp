#include "State_Intro.h"
#include "StateManager.h"
#include "EventManager.h"
#include "SpriteSheet.h"
#include "Window.h"
#include "SharedContext.h"
#include <memory>

/*State_Intro::State_Intro(StateManager* l_stateManager)
	: BaseState(l_stateManager), m_introTexture{ []() {
	sf::Texture texture;
	if (!texture.loadFromFile("preview.gif")) [[unlikely]] {
		std::println("[ERROR] Картинка заставки Intro.png не знайдена в папці проекту!");
		exit(EXIT_FAILURE);
	}
	return texture;
}() }, m_introSprite{ m_introTexture }, m_font{ []() {
	sf::Font font;
	if (!font.openFromFile("C:\\Windows\\Fonts\\Arial.ttf")) [[unlikely]] {
		std::println("[ERROR] Системний шрифт Arial не знайдено в Windows!");
		exit(EXIT_FAILURE);
	}
	return font;
}() }, m_text{ m_font }, m_timePassed{ 0.0f }
{}

void State_Intro::onCreate() {
	// Отримуємо живий розмір вікна через SharedContext
	sf::Vector2u windowSize = m_stateManager->getSharedContext()->m_window->getWindowSize();

	m_introSprite.setOrigin({ m_introTexture.getSize().x / 2.0f, m_introTexture.getSize().y / 2.0f });
	m_introSprite.setPosition({ windowSize.x / 2.0f, 0.0f }); // Ховаємо гриб нагорі екрана

	// Налаштовуємо та центруємо текст за стандартом векторів
	std::pmr::string strForSetString{ "Press SPACE to continue", LocalMemoryManager::getManager().getAlloc() };
	m_text.setString(std::string_view(strForSetString));
	m_text.setCharacterSize(15u);

	sf::FloatRect textRect = m_text.getLocalBounds();
	m_text.setOrigin({ textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f });
	m_text.setPosition({ windowSize.x / 2.0f, windowSize.y / 1.8f });

	// Реєструємо Пробіл строго один раз у кімнату заставки (Intro)
	EventManager* eventManager = m_stateManager->getSharedContext()->m_eventManager;
	eventManager->addCallback(BaseState::StateType::Intro, "Intro_continueCustom", &State_Intro::continueCustom, this);
}

void State_Intro::onDestroy() {
	EventManager* eventManager = m_stateManager->getSharedContext()->m_eventManager;
	eventManager->removeCallback(BaseState::StateType::Intro, "Intro_continueCustom");
}

void State_Intro::update(const sf::Time l_time) {
	if (m_timePassed < 5.0f) { // Less than five seconds.
		m_timePassed += l_time.asSeconds();
		m_introSprite.setPosition({ m_introSprite.getPosition().x,
			m_introSprite.getPosition().y + (100.0f * l_time.asSeconds()) });
	}
}

void State_Intro::draw() {
	sf::RenderWindow* window = m_stateManager->getSharedContext()->m_window->getRenderWindow();

	window->draw(m_introSprite);

	// Подовжуємо ефект автора: текст "Press SPACE" з'являється тільки через 5 секунд!
	if (m_timePassed >= 5.0f) {
		window->draw(m_text);
	}
}

void State_Intro::continueCustom(EventDetails* l_details) {
	// «ЕФЕКТ ДОМІНО»: при натисканні Пробілу перемикаємо рантайм на Головне Меню
	if (m_timePassed >= 5.0f) {
		m_stateManager->switchTo(BaseState::StateType::MainMenu);
		m_stateManager->remove(BaseState::StateType::Intro);
	}
}

void State_Intro::activate() {}
void State_Intro::deactivate() {}
*/

State_Intro::State_Intro(StateManager* l_stateManager)
	: BaseState(l_stateManager)
	, m_spriteSheet{ nullptr }
	, m_font{ []() {
		sf::Font font;
		if (!font.openFromFile("C:\\Windows\\Fonts\\Arial.ttf")) { std::exit(EXIT_FAILURE); }
		return font;
	  }() }
	, m_text{ m_font }
	, m_timePassed{ 0.0f }
{}

void State_Intro::onCreate() {
	sf::Vector2u windowSize = m_stateManager->getSharedContext()->m_window->getWindowSize();

	// Виділяємо пам'ять під спрайт-шит заставки на нашій PMR-арені!
	auto alloc = LocalMemoryManager::getManager().getAlloc();
	void* rawMemory = alloc.allocate(sizeof(SpriteSheet));
	m_spriteSheet = std::construct_at(reinterpret_cast<SpriteSheet*>(rawMemory), m_stateManager->getSharedContext()->m_textureManager);

	// Завантажуємо створений у Photopea конфіг
	m_spriteSheet->loadSheet("IntroSheet.cfg");
	m_spriteSheet->setAnimation("Play", true, true);

	// Центруємо анімацію на екрані
	sf::Sprite* sprite = m_spriteSheet->getSprite();
	if (sprite) {
		const sf::Vector2i frameSize{ m_spriteSheet->getSpriteSize() };
		sprite->setOrigin({ 0.0f, 0.0f });
		sprite->setPosition({ 0.0f, 0.0f });
		sprite->setScale({
			static_cast<float>(windowSize.x) / static_cast<float>(frameSize.x),
			static_cast<float>(windowSize.y) / static_cast<float>(frameSize.y)
		});
	}

	// Налаштовуємо текст... (твій старий код Press SPACE)
	/*std::pmr::string strForSetString{"Press any button to continue", LocalMemoryManager::getManager().getAlloc()};
	m_text.setString(std::string_view(strForSetString));
	m_text.setCharacterSize(15u);
	sf::FloatRect textRect = m_text.getLocalBounds();
	m_text.setOrigin({ textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f });
	m_text.setPosition({ windowSize.x / 2.0f, windowSize.y / 1.1f });*/

	m_stateManager->getSharedContext()->m_eventManager->addCallback(BaseState::StateType::Intro, "Intro_continueCustom", &State_Intro::continueCustom, this);
}

void State_Intro::update(const sf::Time l_time) {
	m_timePassed += l_time.asSeconds();

	// ОЖИВЛЯЄМО КАДРИ: Твій менеджер анімацій сам почне крутити лицаря та напис!
	if (m_spriteSheet) {
		m_spriteSheet->update(l_time.asSeconds());
		sf::Sprite* sprite{ m_spriteSheet->getSprite() };
		if (sprite) {
			const sf::Vector2u windowSize{ m_stateManager->getSharedContext()->m_window->getWindowSize() };
			const sf::Vector2i frameSize{ m_spriteSheet->getSpriteSize() };
			sprite->setOrigin({ 0.0f, 0.0f });
			sprite->setPosition({ 0.0f, 0.0f });
			sprite->setScale({
				static_cast<float>(windowSize.x) / static_cast<float>(frameSize.x),
				static_cast<float>(windowSize.y) / static_cast<float>(frameSize.y)
			});
		}
	}

	if (m_timePassed >= 5.0f) {
		m_stateManager->switchTo(BaseState::StateType::MainMenu);
		m_stateManager->remove(BaseState::StateType::Intro);
	}
}

void State_Intro::draw() {
	sf::RenderWindow* window = m_stateManager->getSharedContext()->m_window->getRenderWindow();

	// Малюємо поточний анімований кадр заставки
	if (m_spriteSheet) {
		m_spriteSheet->draw(window);
	}

	if (m_timePassed >= 5.0f) {
		window->draw(m_text);
	}
}

void State_Intro::onDestroy() {
	m_stateManager->getSharedContext()->m_eventManager->removeCallback(BaseState::StateType::Intro, "Intro_continueCustom");

	// Чисто звільняємо пам'ять заставки з PMR-арени
	if (m_spriteSheet) {
		std::destroy_at(m_spriteSheet);
		LocalMemoryManager::getManager().getAlloc().deallocate(reinterpret_cast<std::byte*>(m_spriteSheet), sizeof(SpriteSheet));
		m_spriteSheet = nullptr;
	}
}

void State_Intro::continueCustom(EventDetails* l_details) {
	// «ЕФЕКТ ДОМІНО»: при натисканні Пробілу перемикаємо рантайм на Головне Меню
	m_stateManager->switchTo(BaseState::StateType::MainMenu);
	m_stateManager->remove(BaseState::StateType::Intro);
}

void State_Intro::activate() {}
void State_Intro::deactivate() {}