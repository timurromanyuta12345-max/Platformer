#include "Enemy.h"
#include "EntityManager.h"
#include "Map.h"
#include "SharedContext.h"
#include <cstdlib>
#include <cmath>

Random Enemy::engine{};

Enemy::Enemy(EntityManager* l_manager) : Character(l_manager), m_hasDestination{ false }, m_chaseForPlayer{ false } {
	m_type = EntityType::Enemy;
	if (engine.chase(engine.m_mt) == 1) {
		m_chaseForPlayer = true;
	}
}

void Enemy::onEntityCollision(EntityBase* l_collider, bool l_attack) {
	if (m_state == EntityState::Dying || m_state == EntityState::Attacking || m_state == EntityState::Hurt) [[unlikely]] {
		return;
	}

	if (l_attack) {
		return;
	}

	if (l_collider->getType() != EntityType::Player) {
		return;
	}

	Character* player{ static_cast<Character*>(l_collider) };
	setState(EntityState::Attacking);
	player->getHurt(1u);
	const bool playerIsLeft{ m_position.x > player->getPosition().x };
	player->addVelocity(playerIsLeft ? -80.0f : 80.0f, 0.0f);
	if (playerIsLeft) {
		m_spritesheet.setDirection(Direction::Left);
	}
	else {
		m_spritesheet.setDirection(Direction::Right);
	}
}

void Enemy::update(float l_dt) {
	Map* map{ m_entityManager->getContext()->m_gameMap };

	if (!map) [[unlikely]] {
		return;
	}
	const bool onGround{ m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnGround)] || (m_velocity.y == 0.0f && m_referenceTile) };

	if (onGround && m_state != EntityState::Jumping && m_state != EntityState::Dying && m_state != EntityState::Hurt && m_state != EntityState::Attacking) {
		const int tileSize{ static_cast<int>(map->getTileSize()) };
		const int direction{ m_velocity.x > 0.0f ? 1 : m_velocity.x < 0.0f ? -1 : (m_spritesheet.getDirection() == Direction::Right ? 1 : -1) };
		const int firstTileX{ direction > 0 ? static_cast<int>(std::ceil((m_AABB.position.x + m_AABB.size.x) / tileSize)) : static_cast<int>(std::floor((m_AABB.position.x - 1.0f) / tileSize)) };
		const int firstTileY{ static_cast<int>(std::floor(m_AABB.position.y / tileSize)) };
		const int lastTileY{ static_cast<int>(std::floor((m_AABB.position.y + m_AABB.size.y - 1.0f) / tileSize)) + 4 };
		const sf::Vector2u mapSize{ map->getMapSize() };

		bool deadlyTileAhead{ false };
		for (int distance{ 0 }; distance < 2 && !deadlyTileAhead; ++distance) {
			const int tileX{ firstTileX + direction * distance };
			if (tileX < 0 || tileX >= static_cast<int>(mapSize.x)) {
				continue;
			}

			for (int tileY{ firstTileY }; tileY <= lastTileY; ++tileY) {
				if (tileY < 0 || tileY >= static_cast<int>(mapSize.y)) {
					continue;
				}

				Tile* tile{ map->getTile(static_cast<uint32_t>(tileX), static_cast<uint32_t>(tileY)) };
				if (tile && tile->m_properties && tile->m_properties->m_deadly) {
					deadlyTileAhead = true;
					break;
				}
			}
		}

		if (deadlyTileAhead) {
			jump();
		}
	}
	Character::update(l_dt);
	const bool collidedX{ m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnX)] };
	const bool onGroundAfterUpdate{ m_flags[static_cast<size_t>(Axis_Collisions::CollidingOnGround)] };
	if (collidedX && onGroundAfterUpdate && m_state != EntityState::Jumping && m_state != EntityState::Dying && m_state != EntityState::Hurt && m_state != EntityState::Attacking) {
		jump();
	}

	if (m_chaseForPlayer) {
		EntityBase* player = m_entityManager->find("Player");

		if (player && m_state != EntityState::Dying && m_state != EntityState::Hurt && m_state != EntityState::Attacking) {
			const float deltaX = player->getPosition().x - m_position.x;
			const float verticalGap = m_position.y - player->getPosition().y;
			const float gravity = map->getGravity();
			const float maxJumpHeight = gravity > 0.0f ? (m_jumpVelocity * m_jumpVelocity) / (2.0f * gravity) : 0.0f;
			const float tileSize = static_cast<float>(map->getTileSize());

			if (onGroundAfterUpdate && verticalGap >= tileSize * 0.5f && verticalGap <= maxJumpHeight && std::abs(deltaX) <= tileSize * 3.0f) {
				jump();
			}

			if (std::abs(deltaX) > 10.0f) {
				move(deltaX > 0.0f ? Direction::Right : Direction::Left);
			}
		}

		return;
	}

	if (m_hasDestination) {
		if (std::abs(m_destination.x - m_position.x) < 16.0f) [[unlikely]] {
			m_hasDestination = false;
			return;
		}

		if (m_destination.x - m_position.x > 0.0f) {
			move(Direction::Right);
		}
		else {
			move(Direction::Left);
		}

		return;
	}

	int random{ engine.go_or_stay(engine.m_mt) };
	if (random != 120) [[likely]] {
		return;
	}

	int newX{ engine.distance(engine.m_mt) };
	if (engine.direction(engine.m_mt)) {
		newX = -newX;
	}

	m_destination.x = m_position.x + newX;
	if (m_destination.x < 0.0f) [[unlikely]] {
		m_destination.x = 0.0f;
	}
	else if (uint32_t maxMapX = map->getMapSize().x * map->getTileSize(); m_destination.x > maxMapX) [[unlikely]] {
		m_destination.x = maxMapX;
	}

	m_hasDestination = true;
}

size_t Enemy::getSizeObject() const noexcept {
	return sizeof(Enemy);
}