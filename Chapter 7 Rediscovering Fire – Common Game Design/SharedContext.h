#pragma once

class Map;
class Window;
class EventManager;
class TextureManager;
class EntityManager;

// Глобальный конверт-паспорт систем игры
struct SharedContext {
	::Window* m_window;
	EventManager* m_eventManager;
	TextureManager* m_textureManager;
	Map* m_gameMap{ nullptr };
	EntityManager* m_entityManager;

	SharedContext() : m_window{ nullptr }, m_eventManager{ nullptr }, m_textureManager{ nullptr }, m_gameMap{ nullptr }, m_entityManager{ nullptr } {}
};