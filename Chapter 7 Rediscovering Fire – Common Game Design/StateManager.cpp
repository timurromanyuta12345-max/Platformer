#include "StateManager.h"
#include "State_Intro.h"
#include "State_MainMenu.h"
#include "State_Game.h"
#include "State_Paused.h"
#include "SharedContext.h"
#include "Window.h"

StateManager::StateManager(SharedContext* l_contextShared)
	: m_sharedContext{ l_contextShared }
	, m_states{ LocalMemoryManager::getManager().getAlloc() }
	, m_toRemove{ LocalMemoryManager::getManager().getAlloc() }
	, m_stateFactory{ LocalMemoryManager::getManager().getAlloc() }
{
	m_stateFactory.max_load_factor(0.5f);
	m_states.reserve(6uz);
	m_toRemove.reserve(6uz);
	m_stateFactory.reserve(6uz);

	registerState<State_Intro>(BaseState::StateType::Intro);
	registerState<State_MainMenu>(BaseState::StateType::MainMenu);
	registerState<State_Game>(BaseState::StateType::Game);
	registerState<State_Paused>(BaseState::StateType::Paused);
}

// Идеальный деструктор, который вычищает PMR-пулы без единого вылета!
StateManager::~StateManager() {
	// СНАЧАЛА ВЫЖИГАЕМ ФАБРИКУ ЛЯМБД! 
	m_stateFactory.clear();
	m_toRemove.clear();

	// И только теперь спокойно и безопасно уничтожаем активные состояния кадра!
		while (!m_states.empty()) {
			if (m_states.back().state) [[likely]] {
				m_states.back().state->onDestroy();
				// вызов деструктора и освобождение памяти из PMR-аллокатора
				std::destroy_at(m_states.back().state);
				LocalMemoryManager::getManager().deallocate(m_states.back().state, m_states.back().allocBytes);
			}
			m_states.pop_back();
		}
}

void StateManager::draw() {
	if (m_states.empty()) [[unlikely]] { 
		return; 
	}
	
	if (m_states.back().state->isTransparent() && m_states.size() > 1uz) [[likely]] {
		size_t i = m_states.size() - 1uz;
		for (; i > 0uz; --i) {
			if (!m_states[i].state->isTransparent()) [[likely]] {
				break;
			}
		}
		for (; i < m_states.size(); ++i) {
			m_sharedContext->m_window->getRenderWindow()->setView(m_states[i].state->getView());
			m_states[i].state->draw();
		}
	}
	else [[unlikely]] {
		m_sharedContext->m_window->getRenderWindow()->setView(m_states.back().state->getView());
		m_states.back().state->draw();
	}
}

void StateManager::update(const sf::Time l_time) {
	if (m_states.empty()) [[unlikely]] {
		return;
	}

	if (m_states.back().state->isTranscendent() && m_states.size() > 1uz) [[likely]] {
		size_t i = m_states.size() - 1uz;
		for (; i > 0uz; --i) {
			if (!m_states[i].state->isTranscendent()) [[likely]] {
				break;
			}
		}
		for (; i < m_states.size(); ++i) {
			m_states[i].state->update(l_time);
		}
	}
	else [[unlikely]] {
		m_states.back().state->update(l_time);
	}
}

bool StateManager::hasState(const BaseState::StateType l_type) {
	for (const auto& entry : m_states) {
		if (static_cast<BaseState::StateType>(entry.type) == l_type) [[unlikely]] {
			auto removed = std::find(m_toRemove.begin(), m_toRemove.end(), l_type);
			if (removed == m_toRemove.end()) [[unlikely]] {
				return true;
			}
			return false;
		}
	}
	return false;
}

void StateManager::remove(const BaseState::StateType l_type) {
	m_toRemove.push_back(l_type);
}

void StateManager::processRequests() {
	while (!m_toRemove.empty()) {
		removeState(m_toRemove.back());
		m_toRemove.pop_back();
	}
}

void StateManager::switchTo(const BaseState::StateType l_type) {
	m_sharedContext->m_eventManager->setCurrentState(l_type);

	for (auto itr{ m_states.begin() }; itr != m_states.end(); ++itr) {
		if (static_cast<BaseState::StateType>(itr->type) == l_type) [[unlikely]] {
			// Деактивируем текущий задний стейт и перемещаем найденный стейт на его место
			m_states.back().state->deactivate();

			// Перемещаем найденный элемент в конец: поменяем type, state pointer и allocBytes
			if (itr != std::prev(m_states.end())) {
				std::swap(itr->type, m_states.back().type);
				std::swap(itr->state, m_states.back().state);
				std::swap(itr->allocBytes, m_states.back().allocBytes);
				// Активируем тот, что теперь в конце
				m_states.back().state->activate();
				m_sharedContext->m_window->getRenderWindow()->setView(m_states.back().state->getView());
				return;
			}

			// Если найденный элемент и так последний — просто реактивируем его.
			m_states.back().state->activate();
			return;
		}
	}

	if (!m_states.empty()) [[likely]] {
		m_states.back().state->deactivate();
	}
	createState(l_type);
	m_states.back().state->activate();
	m_sharedContext->m_window->getRenderWindow()->setView(m_states.back().state->getView());
}

void StateManager::createState(const BaseState::StateType l_type) {
	auto newState = m_stateFactory.find(static_cast<uint8_t>(l_type));
	if (newState == m_stateFactory.end()) [[unlikely]] {
		return;
	}

	auto res = newState->second(); // pair<BaseState*, size>
	BaseState* state = res.first;
	state->m_view = m_sharedContext->m_window->getRenderWindow()->getDefaultView();
	std::size_t bytes = res.second;
	m_states.emplace_back(static_cast<uint8_t>(l_type), state, bytes);
	m_states.back().state->onCreate();
}

void StateManager::removeState(const BaseState::StateType l_type) {
	for (auto itr = m_states.begin(); itr != m_states.end(); ++itr) {
		if (static_cast<BaseState::StateType>(itr->type) == l_type) [[unlikely]] {
			itr->state->onDestroy();

			// вызов деструктора и освобождение памяти из PMR-аллокатора
			std::destroy_at(itr->state);
			LocalMemoryManager::getManager().deallocate(itr->state, itr->allocBytes);

			// Перемещаем последний элемент в позицию итератора и удаляем последний
			if (itr != std::prev(m_states.end())) {
				itr->type = m_states.back().type;
				itr->state = m_states.back().state;
				itr->allocBytes = m_states.back().allocBytes;
			}
			m_states.pop_back();
			return;
		}
	}
}