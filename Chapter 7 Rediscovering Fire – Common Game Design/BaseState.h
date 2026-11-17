#pragma once
#include <SFML/Graphics.hpp>
#include <bitset>

// Абстрактный базовый класс для всех экранов игры
class BaseState {
private:
	friend class StateManager;
	enum class Flags : uint8_t { Transparent, Transcendent, Count };
	enum class StateType : uint8_t { Intro = 1, MainMenu, Game, Paused, GameOver, Credits, Count };
protected:
	StateManager* m_stateManager;
	std::bitset<static_cast<size_t>(Flags::Count)> m_flags;
	sf::View m_view;
public:
	BaseState(StateManager* l_stateManager) noexcept
		: m_stateManager{ l_stateManager }
		, m_flags{ false }
	{}

	virtual ~BaseState() noexcept = default;

	virtual void onCreate() = 0;
	virtual void onDestroy() = 0;
	virtual void activate() = 0;
	virtual void deactivate() = 0;
	virtual void update(const sf::Time l_time) = 0;
	virtual void draw() = 0;

	__forceinline void setTransparent(const bool l_transparent) noexcept {
		m_flags[(static_cast<size_t>(Flags::Transparent))] = l_transparent;
	}

	__forceinline bool isTransparent() const noexcept {
		return m_flags[(static_cast<size_t>(Flags::Transparent))];
	}

	__forceinline void setTranscendent(const bool l_transcendent) noexcept {
		m_flags[(static_cast<size_t>(Flags::Transcendent))] = l_transcendent;
	}

	__forceinline bool isTranscendent() const noexcept {
		return m_flags[(static_cast<size_t>(Flags::Transcendent))];
	}

	__forceinline StateManager* getStateManager() const noexcept {
		return m_stateManager;
	}

	__forceinline sf::View& getView() noexcept {
		return m_view;
	}
};