#pragma once
#include "MemoryManager.h"
#include <unordered_map>
#include <string>
#include <cstdint>
#include <functional>
#include <vector>
#include <string_view>
#include <type_traits>

class EntityBase;
class Character;
enum class EntityType : uint8_t;	
struct SharedContext;

using EntityContainer = std::pmr::unordered_map<uint32_t, EntityBase*>;
using EntityFactory = std::pmr::unordered_map<EntityType, std::move_only_function<EntityBase* (void)>>;

struct TransparentStringHash {
	using is_transparent = void;

	size_t operator()(std::string_view sv) const noexcept {
		return std::hash<std::string_view>{}(sv);
	}

	size_t operator()(std::pmr::string& s) const noexcept {
		return std::hash<std::pmr::string>{}(s);
	}

	size_t operator()(const char* str) const noexcept {
		return std::hash<std::string_view>{}(str);
	}
};

using EnemyTypes = std::pmr::unordered_map<std::pmr::string, std::pmr::string, TransparentStringHash, std::equal_to<>>;

class EntityManager {
private:
	EntityContainer m_entities;
	EnemyTypes m_enemyTypes;
	EntityFactory m_entityFactory;
	SharedContext* m_context;
	uint32_t m_idCounter;
	uint32_t m_maxEntities;
	std::pmr::vector<uint32_t> m_entitiesToRemove;

	// Шаблонный PMR-аллокатор
	template<class T>
	T* allocateEntity(EntityManager* l_manager) {
		void* rawMemory = LocalMemoryManager::getManager().getAlloc().allocate(sizeof(T));
		T* stateAddress = std::construct_at(reinterpret_cast<T*>(rawMemory), l_manager);
		return stateAddress;
	}

	template<class T>
	void registerEntity(const EntityType& l_type) {
		m_entityFactory[l_type] = [this]() -> EntityBase* {
			return allocateEntity<T>(this);
			};
	}

	void processRemovals();
	void loadEnemyTypes(const std::string_view l_name);
	void entityCollisionCheck();
public:
	EntityManager(SharedContext* l_context, uint32_t l_maxEntities);
	~EntityManager();

	int add(const EntityType l_type, const std::string_view l_name = "");
	EntityBase* find(uint32_t l_id);
	EntityBase* find(const std::string_view l_name);
	void remove(uint32_t l_id);
	void update(float l_dt);
	void draw();
	void purge();
	__forceinline SharedContext* getContext() {
		return m_context;
	}
};