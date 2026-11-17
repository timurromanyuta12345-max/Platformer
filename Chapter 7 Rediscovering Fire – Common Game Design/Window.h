#pragma once
#include "EventManager.h"
#include <bitset>

struct EventDetails;

class Window {
public:
	enum class WindowFlags : uint8_t { isDone = 0, isFullscreen = 1, isFocused = 2, count = 3 };
private:
	sf::RenderWindow m_window;
	sf::Vector2u m_windowSize;
	sf::String m_windowTitle;
	std::bitset<static_cast<size_t>(WindowFlags::count)> m_windowFlags;
	EventManager m_eventManagerW;

	void setup(const sf::String& l_title, const sf::Vector2u& l_size);
	void destroy();
	void create();
public:
	Window();
	Window(const sf::String& l_title, const sf::Vector2u& l_size);
	Window(const Window& l_window) = delete;
	Window& operator=(const Window& l_window) = delete;
	Window(Window&& l_window) = delete;
	Window& operator=(Window&& l_window) = delete;
	~Window();

	void beginDraw();
	void endDraw();
	void draw(sf::Drawable& l_drawable);

	__forceinline bool isDone() {
		return m_windowFlags[(static_cast<size_t>(Window::WindowFlags::isDone))];
	}

	__forceinline bool isFullscreen() {
		return m_windowFlags[(static_cast<size_t>(Window::WindowFlags::isFullscreen))];
	}

	__forceinline bool isFocused() {
		return m_windowFlags[(static_cast<size_t>(Window::WindowFlags::isFocused))];
	}

	__forceinline sf::Vector2u getWindowSize() {
		return m_windowSize;
	}

	__forceinline sf::RenderWindow& getWindow() {
		return m_window;
	}

	__forceinline EventManager* getEventManager() {
		return &m_eventManagerW;
	}

	__forceinline void close(EventDetails* l_details = nullptr) {
		m_windowFlags[(static_cast<size_t>(Window::WindowFlags::isDone))] = true;
	}

	__forceinline sf::RenderWindow* getRenderWindow() {
		return &m_window;
	}

	void toggleFullscreen(EventDetails* l_details);

	void update();
	void updateInput();

	sf::FloatRect getViewSpace();
};