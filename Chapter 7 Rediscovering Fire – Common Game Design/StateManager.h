#pragma once
#include "BaseState.h"
#include "MemoryManager.h"
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>
#include <functional>

struct SharedContext;

// Штатное хранение состояний: сырой указатель, выделенный из PMR-арены
template<typename T>
using State = T*; // raw pointer allocated from PMR arena

// Структура состояния: хранит тип (uint8_t) и уникальный указатель с делейтером
struct StateEntry {
	uint8_t type;
	BaseState* state;
	std::size_t allocBytes;

	StateEntry(uint8_t t, BaseState* s, std::size_t b)
		: type(t), state(s), allocBytes(b) {}

	StateEntry(StateEntry&&) noexcept = default;
	StateEntry& operator=(StateEntry&&) noexcept = default;

	// delete copy to prevent accidental copies
	StateEntry(const StateEntry&) = delete;
	StateEntry& operator=(const StateEntry&) = delete;
};

using StateContainer = std::pmr::vector<StateEntry>;
using TypeContainer = std::pmr::vector<BaseState::StateType>;
using StateFactory = std::pmr::unordered_map<uint8_t, std::move_only_function<std::pair<BaseState*, std::size_t>()>>;

class StateManager {
private:
	SharedContext* m_sharedContext;
	StateContainer m_states;
	TypeContainer m_toRemove;
	StateFactory m_stateFactory;

	void createState(const BaseState::StateType l_type);
	void removeState(const BaseState::StateType l_type);

public:
	StateManager(SharedContext* l_sharedContext);
	~StateManager();

	// Шаблонный PMR-аллокатор
	template<class T>
	State<T> allocateState(StateManager* l_manager) {
		void* rawMemory = LocalMemoryManager::getManager().getAlloc().allocate(sizeof(T));
		T* stateAddress = std::construct_at(reinterpret_cast<T*>(rawMemory), l_manager);
		return stateAddress;
	}

	// Шаблонный метод регистрации с контролируемым полиморфным приведением
	template<class T>
	void registerState(const BaseState::StateType l_type) {
		m_stateFactory[static_cast<uint8_t>(l_type)] = [this]() -> std::pair<BaseState*, std::size_t> {
			BaseState* rawBase = allocateState<T>(this);
			return { rawBase, sizeof(T) };
		};
	}

	void update(const sf::Time l_time);
	void draw();
	void processRequests();
	void switchTo(const BaseState::StateType l_type);
	void remove(const BaseState::StateType l_type);

	__forceinline SharedContext* getSharedContext() noexcept {
		return m_sharedContext;
	}

	bool hasState(const BaseState::StateType l_type);
};