#include "Window.h"

::Window::Window() {
	setup("Chapter 7", { 1920u, 1080u });
}

::Window::Window(const sf::String& l_title, const sf::Vector2u& l_size) {
	setup(l_title, l_size);
}

::Window::~Window() {
	destroy();
}

void ::Window::setup(const sf::String& l_title, const sf::Vector2u& l_size) {
	m_windowTitle = l_title;
	m_windowSize = l_size;
	m_windowFlags[static_cast<size_t>(WindowFlags::isFullscreen)] = false;
	m_windowFlags[static_cast<size_t>(WindowFlags::isDone)] = false;
	m_windowFlags[static_cast<size_t>(WindowFlags::isFocused)] = true;

	m_eventManagerW.addCallback(BaseState::StateType(0), "Fullscreen_toggle", &::Window::toggleFullscreen, this);
	m_eventManagerW.addCallback(BaseState::StateType(0), "Window_close", &::Window::close, this);
	create();
}

void ::Window::create() {
	const bool isFullscreen{ m_windowFlags[static_cast<size_t>(WindowFlags::isFullscreen)] };
	auto state = isFullscreen ? sf::State::Fullscreen : sf::State::Windowed;
	m_window.create(sf::VideoMode({ m_windowSize.x, m_windowSize.y }), m_windowTitle, state);
	//m_window.setVerticalSyncEnabled(true);
}

void ::Window::destroy() {
	m_window.close();
}

void ::Window::beginDraw() {
	m_window.clear(sf::Color(30, 30, 30));
}

void ::Window::endDraw() {
	m_window.display();
}

void ::Window::draw(sf::Drawable& l_drawable) {
	m_window.draw(l_drawable);
}

void Window::toggleFullscreen(EventDetails* l_details) {
	m_windowFlags.flip(static_cast<size_t>(WindowFlags::isFullscreen));
	bool isFullscreen = m_windowFlags[static_cast<size_t>(WindowFlags::isFullscreen)];

	m_windowFlags[static_cast<size_t>(WindowFlags::isFocused)] = false;
	m_eventManagerW.setFocus(false);

	sf::VideoMode mode(sf::Vector2u(m_windowSize.x, m_windowSize.y));
	if (isFullscreen) {
		auto fullscreenModes = sf::VideoMode::getFullscreenModes();
		if (!fullscreenModes.empty()) [[likely]] {
			mode = fullscreenModes[0];
		}
	}

	m_window.create(mode, m_windowTitle, isFullscreen ? sf::State::Fullscreen : sf::State::Windowed);
	//m_window.setVerticalSyncEnabled(true);

	m_windowFlags[static_cast<size_t>(WindowFlags::isFocused)] = true;
	m_eventManagerW.setFocus(true);
}

void ::Window::update() {
	while (const std::optional<sf::Event> event = m_window.pollEvent()) {
		if (event->is<sf::Event::Closed>()) [[unlikely]] {
			m_windowFlags[static_cast<size_t>(WindowFlags::isDone)] = true;
		}
		else if (event->is<sf::Event::FocusLost>()) [[unlikely]] {
			m_windowFlags[static_cast<size_t>(WindowFlags::isFocused)] = false;
			m_eventManagerW.setFocus(false);
		}
		else if (event->is<sf::Event::FocusGained>()) [[unlikely]] {
			m_windowFlags[static_cast<size_t>(WindowFlags::isFocused)] = true;
			m_eventManagerW.setFocus(true);
		}

		m_eventManagerW.handleEvent(*event, m_window);
	}
}

void Window::updateInput() {
	m_eventManagerW.update();
}

sf::FloatRect Window::getViewSpace() {
	sf::Vector2f viewCenter{ m_window.getView().getCenter() };
	sf::Vector2f viewSize{ m_window.getView().getSize() };
	sf::Vector2f viewSizeHalf{ viewSize.x / 2.0f, viewSize.y / 2.0f };
	sf::FloatRect viewSpace{ viewCenter - viewSizeHalf, viewSize };
	return viewSpace;
}