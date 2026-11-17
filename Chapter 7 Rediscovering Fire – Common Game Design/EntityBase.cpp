#include "EntityBase.h"
#include "MemoryManager.h"
#include "Map.h"
#include "EntityManager.h"
#include "SharedContext.h"
#include <cstdlib>
#include <algorithm>

EntityBase::EntityBase(EntityManager* l_entityManager) : m_name{ "BaseEntity", LocalMemoryManager::getManager().getAlloc() }, m_type{ EntityType::Base }, m_id{ 0u }, m_referenceTile{ nullptr }, m_state{ EntityState::Idle }, m_flags{ false }, m_collisions{ LocalMemoryManager::getManager().getAlloc() }, m_entityManager { l_entityManager } {}

void EntityBase::setPosition(float l_x, float l_y) {
	m_position = sf::Vector2f{ l_x, l_y };
	updateAABB();
}

void EntityBase::setPosition(sf::Vector2f l_position) {
	m_position = l_position;
	updateAABB();
}

void EntityBase::setSize(sf::Vector2f l_size) {
	m_size = l_size;
	updateAABB();
}

void EntityBase::setState(EntityState l_state) {
	if (m_state == EntityState::Dying) [[unlikely]] {
		return;
	}

	m_state = l_state;
}

void EntityBase::move(float l_x, float l_y) {
	m_positionOld = m_position;
	m_position += sf::Vector2f{ l_x, l_y };
	sf::Vector2u mapSize{ m_entityManager->getContext()->m_gameMap->getMapSize() };
	if (m_position.x < 0.0f) [[unlikely]] {
		m_position.x = 0.0f;
	}
	else if (m_position.x > mapSize.x * Sheet::Tile_Size) [[unlikely]] {
		m_position.x = mapSize.x * Sheet::Tile_Size;
	}

	if (m_position.y < 0.0f) [[unlikely]] {
		m_position.y = 0.0f;
	}
	else if (m_position.y > mapSize.y * Sheet::Tile_Size) [[unlikely]] {
		m_position.y = mapSize.y * Sheet::Tile_Size;
		setState(EntityState::Dying);
	}

	updateAABB();
}

void EntityBase::addVelocity(float l_x, float l_y) {
	m_velocity += sf::Vector2f{ l_x, l_y };
	if (std::abs(m_velocity.x) > m_maxVelocity.x) [[unlikely]] {
		if (m_velocity.x < 0.0f) [[unlikely]] {
			m_velocity.x = -m_maxVelocity.x;
		}
		else [[likely]] {
			m_velocity.x = m_maxVelocity.x;
		}
	}

	if (std::abs(m_velocity.y) > m_maxVelocity.y) [[unlikely]] {
		if (m_velocity.y < 0.0f) [[unlikely]] {
			m_velocity.y = -m_maxVelocity.y;
		}
		else [[likely]] {
			m_velocity.y = m_maxVelocity.y;
		}
	}
}

void EntityBase::accelerate(float l_x, float l_y) {
	m_acceleration += sf::Vector2f{ l_x, l_y };
}

void EntityBase::applyFriction(float l_x, float l_y) {
	if (m_velocity.x != 0.0f) [[likely]] {
		if ((std::abs(m_velocity.x) - std::abs(l_x)) < 0.0f) [[unlikely]] {
			m_velocity.x = 0.0f;
		}
		else [[likely]] {
			if (m_velocity.x < 0.0f) [[unlikely]] {
				m_velocity.x += l_x;
			}
			else [[likely]] {
				m_velocity.x -= l_x;
			}
		}
	}

	if (m_velocity.y != 0.0f) [[unlikely]] {
		if ((std::abs(m_velocity.y) - std::abs(l_y)) < 0.0f) [[unlikely]] {
			m_velocity.y = 0.0f;
		}
		else [[likely]] {
			if (m_velocity.y < 0.0f) [[unlikely]] {
				m_velocity.y += l_y;
			}
			else [[unlikely]] {
				m_velocity.y -= l_y;
			}
		}
	}
}

void EntityBase::update(float l_dt) {
	Map* map{ m_entityManager->getContext()->m_gameMap };
	float gravity{ map->getGravity() };

	accelerate(0, gravity);
	addVelocity(m_acceleration.x * l_dt, m_acceleration.y * l_dt);
	setAcceleration({ 0.0f, 0.0f });

	sf::Vector2f frictionValue{};
	if (m_referenceTile) [[likely]] {
		frictionValue = m_referenceTile->m_friction;
		if (m_referenceTile->m_deadly) [[unlikely]] {
			setState(EntityState::Dying);
		}
	}

	float friction_x{ (m_speed.x * frictionValue.x) * l_dt };
	float friction_y{ (m_speed.y * frictionValue.y) * l_dt };
	applyFriction(friction_x, friction_y);
	sf::Vector2f deltaPos{ m_velocity * l_dt };
	move(deltaPos.x, deltaPos.y);
	m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnX)] = false;
	m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnY)] = false;
	m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnGround)] = false;
	checkCollisions();
	resolveCollisons();
}

void EntityBase::updateAABB() {
	m_AABB = sf::FloatRect({ m_position.x - (m_size.x / 2.0f), m_position.y - m_size.y }, { m_size.x, m_size.y });
}

void EntityBase::checkCollisions() {
	Map* gameMap{ m_entityManager->getContext()->m_gameMap };
	uint32_t tileSize{ gameMap->getTileSize() };
	int fromX = static_cast<int>(std::floor(m_AABB.position.x / tileSize));
	int toX = static_cast<int>(std::floor((m_AABB.position.x + m_AABB.size.x) / tileSize));
	int fromY = static_cast<int>(std::floor(m_AABB.position.y / tileSize));
	int toY = static_cast<int>(std::floor((m_AABB.position.y + m_AABB.size.y) / tileSize));

	for (int x = fromX; x <= toX; ++x) {
		for (int y = fromY; y <= toY; ++y) {
			Tile* tile{ gameMap->getTile(x, y) };
			if (!tile) [[unlikely]] {
				continue;
			}

			sf::FloatRect tileBounds(sf::Vector2f(x * tileSize, y * tileSize), sf::Vector2f(tileSize, tileSize));
			auto intersectionOpt = m_AABB.findIntersection(tileBounds);
			if (intersectionOpt.has_value()) [[likely]] {
				sf::FloatRect intersection = intersectionOpt.value();
				float area{ intersection.size.x * intersection.size.y };
				/*CollisionElement temp(area, tile->m_properties, tileBounds);
				m_collisions.push_back(temp);*/
				m_collisions.emplace_back(area, tile->m_properties, tileBounds);
				if (tile->m_warp && m_type == EntityType::Player) [[unlikely]] {
					gameMap->loadNext();
				}
			}
		}
	}
}

void EntityBase::resolveCollisons() {
	if (!m_collisions.empty()) [[likely]] {
		std::sort(m_collisions.begin(), m_collisions.end(), CollisionElement::sortCollisions);
		Map* gameMap{ m_entityManager->getContext()->m_gameMap };
		uint32_t tileSize{ gameMap->getTileSize() };
		for (auto& itr : m_collisions) {
			if (!m_AABB.findIntersection(itr.m_tileBounds).has_value()) {
				continue;
			}

			float xDiff = (m_AABB.position.x + (m_AABB.size.x / 2)) - (itr.m_tileBounds.position.x + (itr.m_tileBounds.size.x / 2.0f));
			float yDiff = (m_AABB.position.y + (m_AABB.size.y / 2)) - (itr.m_tileBounds.position.y + (itr.m_tileBounds.size.y / 2.0f));
			float resolve = 0.0f;
			if (std::abs(xDiff) > std::abs(yDiff)) {
				if (xDiff > 0.0f) {
					resolve = (itr.m_tileBounds.position.x + tileSize) - m_AABB.position.x;
				}
				else {
					resolve = -((m_AABB.position.x + m_AABB.size.x) - itr.m_tileBounds.position.x);
				}

				move(resolve, 0);
				m_velocity.x = 0;
				m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnX)] = true;
			}
			else {
				if (yDiff > 0.0f) {
					resolve = (itr.m_tileBounds.position.y + tileSize) - m_AABB.position.y;
				}
				else {
					resolve = -((m_AABB.position.y + m_AABB.size.y) - itr.m_tileBounds.position.y);
					if (!m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnGround)]) {
						m_referenceTile = itr.m_tile;
						m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnGround)] = true;
					}
				}

				move(0, resolve);
				m_velocity.y = 0.0f;
				m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnY)] = true;
			}
		}

		m_collisions.clear();
	}

	if (!m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnGround)]) {
		m_referenceTile = nullptr;
	}
}