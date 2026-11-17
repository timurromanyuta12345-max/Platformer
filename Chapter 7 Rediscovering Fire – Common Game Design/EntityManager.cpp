#include "EntityManager.h"
#include "BaseState.h"
#include "Player.h"
#include "Enemy.h"
#include "SharedContext.h"
#include "Window.h"
#include <memory>
#include <print>
#include <iterator>
#include <fstream>
#include <cstdlib>
#include <string>
#include <spanstream>

EntityManager::EntityManager(SharedContext* l_context, uint32_t l_maxEntities) : m_entities{ LocalMemoryManager::getManager().getAlloc() }, m_enemyTypes{ LocalMemoryManager::getManager().getAlloc() }, m_entityFactory{ LocalMemoryManager::getManager().getAlloc() }, m_context{ l_context }, m_idCounter{ 0u }, m_maxEntities{ l_maxEntities }, m_entitiesToRemove{ LocalMemoryManager::getManager().getAlloc() } {
	loadEnemyTypes("EnemyList.list");
	registerEntity<Player>(EntityType::Player);
	registerEntity<Enemy>(EntityType::Enemy);
}

EntityManager::~EntityManager() {
	purge();
}

int EntityManager::add(const EntityType l_type, const std::string_view l_name) {
	if (m_entities.size() >= m_maxEntities) [[unlikely]] {
		return -1;
	}

	auto itr{ m_entityFactory.find(l_type) };
	if (itr == m_entityFactory.end()) [[unlikely]] {
		return -1;
	}

	EntityBase* entity{ itr->second() };
	entity->m_id = m_idCounter;
	if (l_name != "") [[likely]] {
		entity->m_name = l_name;
	}

	m_entities.emplace(m_idCounter, entity);
	if (l_type == EntityType::Enemy) [[likely]] {
		auto itr{ m_enemyTypes.find(l_name) };
		if (itr != m_enemyTypes.end()) [[likely]] {
			Enemy* enemy{ static_cast<Enemy*>(entity) };
			enemy->load(itr->second);
		}
	}

	++m_idCounter;
	return m_idCounter - 1;
}

EntityBase* EntityManager::find(const std::string_view l_name) {
	for (auto& itr : m_entities) {
		if (itr.second->getName() == l_name) [[unlikely]] {
			return itr.second;
		}
	}

	return nullptr;
}

EntityBase* EntityManager::find(uint32_t l_id) {
	auto itr{ m_entities.find(l_id) };
	if (itr == m_entities.end()) [[unlikely]] {
		return nullptr;
	}

	return itr->second;
}

void EntityManager::remove(uint32_t l_id) {
	m_entitiesToRemove.push_back(l_id);
}

void EntityManager::update(float l_dt) {
	for (auto& itr : m_entities) {
		itr.second->update(l_dt);
	}

	entityCollisionCheck();
	processRemovals();
}

void EntityManager::draw() {
	sf::RenderWindow* window{ m_context->m_window->getRenderWindow() };
	sf::FloatRect viewSpace{ m_context->m_window->getViewSpace() };

	for (auto& itr : m_entities) {
		if (!viewSpace.findIntersection(itr.second->m_AABB)) {
			continue;
		}

		itr.second->draw(window);
	}
}

void EntityManager::purge() {
	for (auto& itr : m_entities) {
		EntityBase* entity{ itr.second };
		const size_t bytes{ entity->getSizeObject() };
		std::destroy_at(entity);
		LocalMemoryManager::getManager().deallocate(entity, bytes);
	}

	m_entities.clear();
	m_idCounter = 0;
}

void EntityManager::processRemovals() {
	while (m_entitiesToRemove.begin() != m_entitiesToRemove.end()) {
		uint32_t id{ m_entitiesToRemove.back() };
		auto itr{ m_entities.find(id) };
		if (itr != m_entities.end()) [[likely]] {
			EntityBase* entity{ itr->second };
			const size_t bytes{ entity->getSizeObject() };
			std::println("Discarding entity: {}", entity->getID());
			std::destroy_at(entity);
			LocalMemoryManager::getManager().deallocate(entity, bytes);
			m_entities.erase(itr);
		}

		m_entitiesToRemove.pop_back();
	}
}

void EntityManager::entityCollisionCheck() {
	if (m_entities.empty()) [[unlikely]] {
		return;
	}

	for (auto itr{ m_entities.begin() }; std::next(itr) != m_entities.end(); ++itr) {
		for (auto itr2{ std::next(itr) }; itr2 != m_entities.end(); ++itr2) {
			if (itr->first == itr2->first) [[unlikely]] {
				continue;
			}

			if (itr->second->m_AABB.findIntersection(itr2->second->m_AABB)) {
				itr->second->onEntityCollision(itr2->second, false);
				itr2->second->onEntityCollision(itr->second, false);
			}

			EntityType t1{ itr->second->getType() };
			EntityType t2{ itr2->second->getType() };
			if (t1 == EntityType::Player || t1 == EntityType::Enemy) [[likely]] {
				Character* c1{ static_cast<Character*>(itr->second) };
				if (c1->m_attackAABB.findIntersection(itr2->second->m_AABB)) [[unlikely]] {
					c1->onEntityCollision(itr2->second, true);
				}
			}

			if (t2 == EntityType::Player || t2 == EntityType::Enemy) [[unlikely]] {
				Character* c2{ static_cast<Character*>(itr2->second) };
				if (c2->m_attackAABB.findIntersection(itr->second->m_AABB)) [[unlikely]] {
					c2->onEntityCollision(itr->second, true);
				}
			}
		}
	}
}

void EntityManager::loadEnemyTypes(const std::string_view l_name) {
	std::pmr::string strForIfstream{ l_name, LocalMemoryManager::getManager().getAlloc() };
	std::ifstream file(strForIfstream.c_str());
	if (!file) [[unlikely]] {
		std::println("The file {} isn't found", l_name);
		std::exit(EXIT_FAILURE);
	}

	std::pmr::string line{ l_name, LocalMemoryManager::getManager().getAlloc() };
	std::pmr::string name{ l_name, LocalMemoryManager::getManager().getAlloc() };
	std::pmr::string charFile{ l_name, LocalMemoryManager::getManager().getAlloc() };
	while (std::getline(file, line)) {
		if (line[0] == '|') [[unlikely]] {
			continue;
		}

		std::spanstream keystream(line);
		keystream >> name >> charFile;
		m_enemyTypes.emplace(name, charFile);
	}

	file.close();
}