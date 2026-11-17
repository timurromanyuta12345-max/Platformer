#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include <utility>
#include <unordered_map>
#include <string_view>
#include <functional>
#include <bitset>
#include <memory>
#include <algorithm>
#include "MemoryManager.h"
#include "BaseState.h"

enum class EventType : uint8_t { 
	KeyDown, // sf::Event::KeyPressed
	KeyUp, // sf::Event::KeyReleased
	MButtonDown, // sf::Event::MouseButtonPressed
	MButtonUp, // sf::Event::MouseButtonReleased
	MouseWheel, // sf::Event::MouseWheelMoved
	WindowResized, // sf::Event::Resized
	GainedFocus, // sf::Event::GainedFocus
	LostFocus, // sf::Event::LostFocus
	MouseEntered, // sf::Event::MouseEntered
	MouseLeft, // sf::Event::MouseLeft
	Closed, // sf::Event::Closed
	TextEntered, // sf::Event::TextEntered
	Keyboard,
	Mouse,
	Joystick,
	Count // in total
};

struct EventInfo {
	std::variant<int> m_code;

	EventInfo() : m_code{ 0 } {};
	EventInfo(int l_event) : m_code{ l_event } {}
};

using Events = std::pmr::vector<std::pair<EventType, EventInfo>>;

struct EventDetails {
	int m_mouseWheelDelta;
	int m_keyCode;
	uint32_t m_textEntered;
	sf::Vector2u m_size;
	sf::Vector2i m_mouse;
	std::pmr::string m_name;

	EventDetails(const std::string_view l_bindName) : m_name{ l_bindName, LocalMemoryManager::getManager().getAlloc() } {
		clear();
	}

	void clear() {
		m_mouseWheelDelta = 0;
		m_keyCode = -1;
		m_textEntered = 0;
		m_size = sf::Vector2u{ 0, 0 };
		m_mouse = sf::Vector2i{ 0, 0 };
	}
};

struct Binding {
	Events m_events;
	std::pmr::string m_name; // own the name to guarantee lifetime
	EventDetails m_details;
	int m_c;

	Binding(const std::string_view l_name, std::pmr::polymorphic_allocator<std::byte> l_alloc)
		: m_events{ l_alloc }                         // Передаем арену в вектор!
		, m_name{ l_name, l_alloc }                    // Передаем арену в PMR-строку!
		, m_details{ m_name }
		, m_c{ 0 }
	{}

	// Binding(const std::string_view l_name) : m_events{ LocalMemoryManager::getManager().getAlloc() }, m_name{ l_name, LocalMemoryManager::getManager().getAlloc() }, m_details{ m_name }, m_c{ 0 } {}

	void bindEvent(EventType l_type, EventInfo l_info = EventInfo()) {
		m_events.emplace_back(l_type, l_info);
	}
};

struct PmrDeleter {
	std::pmr::polymorphic_allocator<std::byte> alloc;

	void operator()(Binding* ptr) const noexcept {
		if (!ptr) [[unlikely]] {
			return;
		}

		std::destroy_at(ptr);

		auto nonConstAlloc = alloc;
		nonConstAlloc.deallocate(reinterpret_cast<std::byte*>(ptr), sizeof(Binding));
	}
};

using PmrBindingPtr = std::unique_ptr<Binding, PmrDeleter>;
using Bindings = std::pmr::unordered_map<std::string_view, PmrBindingPtr>;
using CallbacksContainer = std::pmr::unordered_map<std::string_view, std::move_only_function<void(EventDetails*)>>;
using Callbacks = std::pmr::unordered_map<BaseState::StateType, CallbacksContainer>;

enum class Flags : uint8_t { HasFocus = 0, Count = 1 };

class EventManager {
private:
	std::bitset<static_cast<size_t>(Flags::Count)> m_flags;
	Bindings m_bindings;
	Callbacks m_callbacks;
	BaseState::StateType m_currentState;

	void loadBindings();
public:
	EventManager();
	~EventManager() = default;

	bool addBinding(PmrBindingPtr l_binding);
	bool removeBinding(std::string_view l_name);

	template<typename T>
	bool addCallback(BaseState::StateType l_type, const std::string_view l_name, void(T::* l_func)(EventDetails*), T* l_instance) {
		auto itr{ m_callbacks.emplace(l_type, CallbacksContainer()).first };
		auto l_temp = std::bind_front(l_func, l_instance);
		bool res = itr->second.emplace(l_name, std::move(l_temp)).second;
		return res;
	}

	bool removeCallback(BaseState::StateType l_type, const std::string_view l_name);

	__forceinline void setFocus(const bool l_focus) noexcept {
		m_flags[(static_cast<size_t>(Flags::HasFocus))] = l_focus;
	}
	void handleEvent(const sf::Event& l_event, sf::RenderWindow& lwindow);
	void update();

	sf::Vector2i getMousePosition(sf::RenderWindow* l_window = nullptr) {
		return (l_window ? sf::Mouse::getPosition(*l_window) : sf::Mouse::getPosition());
	}

	__forceinline void setCurrentState(const BaseState::StateType l_type) {
		m_currentState = l_type;
	}
};