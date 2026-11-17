#include "Character.h"
#include "EntityManager.h"
#include "SharedContext.h"
#include "Anim_Base.h"
#include <fstream>
#include <print>
#include <cstdlib>
#include <spanstream>

Character::Character(EntityManager* l_entityManager) : EntityBase(l_entityManager), m_spritesheet{ m_entityManager->getContext()->m_textureManager }, m_jumpVelocity{ 250.0f }, m_hitpoints{ 5 } {
	m_name = "Character";
}

void Character::move(const Direction l_dir) {
	if (getState() == EntityState::Dying) [[unlikely]] {
		return;
	}

	m_spritesheet.setDirection(l_dir);
	if (l_dir == Direction::Left) {
		accelerate(-m_speed.x, 0);
	}
	else {
		accelerate(m_speed.x, 0);
	}

	if (getState() == EntityState::Idle) {
		setState(EntityState::Walking);
	}
}

void Character::jump() {
	if (getState() == EntityState::Dying || getState() == EntityState::Jumping || getState() == EntityState::Hurt) {
		return;
	}

	setState(EntityState::Jumping);
	addVelocity(0, -m_jumpVelocity);
}

void Character::attack() {
	if (getState() == EntityState::Dying || getState() == EntityState::Jumping || getState() == EntityState::Hurt || getState() == EntityState::Attacking) {
		return;
	}

	setState(EntityState::Attacking);
}

void Character::getHurt(const uint32_t l_damage) {
	if (getState() == EntityState::Dying || getState() == EntityState::Hurt) {
		return;
	}

	m_hitpoints = (m_hitpoints - l_damage > 0 ? m_hitpoints - l_damage : 0);
	if (m_hitpoints) {
		setState(EntityState::Hurt);
	}
	else {
		setState(EntityState::Dying);
	}
}

void Character::load(std::string_view l_path) {
	std::pmr::string strForIfstream{ l_path, LocalMemoryManager::getManager().getAlloc() };
	std::ifstream file(strForIfstream.c_str());
	if (!file) [[unlikely]] {
		std::println("The file {} isn't found", l_path);
		std::exit(EXIT_FAILURE);
	}

	std::pmr::string line{ l_path, LocalMemoryManager::getManager().getAlloc() };
	std::pmr::string type{ l_path, LocalMemoryManager::getManager().getAlloc() };
	std::pmr::string path{ l_path, LocalMemoryManager::getManager().getAlloc() };
	while (std::getline(file, line)) {
		std::spanstream keystream(line);
		keystream >> type;
		if (type == "Name") {
			keystream >> m_name;
		}
		else if (type == "Spritesheet") {
			keystream >> path;
			if (!m_spritesheet.loadSheet(path)) {
				std::println("Failed loading spritesheet: {}", path);
				std::exit(EXIT_FAILURE);
			}
			m_spritesheet.setAnimation("Idle", true, true);
		}
		else if (type == "Hitpoints") {
			keystream >> m_hitpoints;
		}
		else if (type == "BoundingBox") {
			sf::Vector2f boundingSize;
			keystream >> boundingSize.x >> boundingSize.y;
			setSize(boundingSize);
		}
		else if (type == "DamageBox") {
			keystream >> m_attackAABBoffset.position.x >> m_attackAABBoffset.position.y >> m_attackAABB.size.x >> m_attackAABB.size.y;
		}
		else if (type == "Speed") {
			keystream >> m_speed.x >> m_speed.y;
		}
		else if (type == "JumpVelocity") {
			keystream >> m_jumpVelocity;
		}

		else if (type == "MaxVelocity") {
			keystream >> m_maxVelocity.x >> m_maxVelocity.y;
		}
		else [[unlikely]] {
			std::println("Unknown type in character file: {}", type);
		}
	}

	file.close();
}

void Character::updateAttackAABB() {
	m_attackAABB.position.x = (m_spritesheet.getDirection() == Direction::Left ? (m_AABB.position.x - m_attackAABB.size.x) - m_attackAABBoffset.position.x : (m_AABB.position.x + m_AABB.size.x) + m_attackAABBoffset.position.x);
	m_attackAABB.position.y = m_AABB.position.y + m_attackAABBoffset.position.y;
}

void Character::animate() {
	EntityState state{ getState() };

	if (state == EntityState::Walking && m_spritesheet.getCurrentAnim()->getName() != "Walk") {
		m_spritesheet.setAnimation("Walk", true, true);
	}
	else if (state == EntityState::Jumping && m_spritesheet.getCurrentAnim()->getName() != "Jump") {
		m_spritesheet.setAnimation("Jump", true, false);
	}
	else if (state == EntityState::Attacking && m_spritesheet.getCurrentAnim()->getName() != "Attack") {
		m_spritesheet.setAnimation("Attack", true, false);
	}
	else if (state == EntityState::Hurt && m_spritesheet.getCurrentAnim()->getName() != "Hurt") {
		m_spritesheet.setAnimation("Hurt", true, false);
	}
	else if (state == EntityState::Dying && m_spritesheet.getCurrentAnim()->getName() != "Death") {
		m_spritesheet.setAnimation("Death", true, false);
	}
	else if (state == EntityState::Idle && m_spritesheet.getCurrentAnim()->getName() != "Idle") {
		m_spritesheet.setAnimation("Idle", true, true);
	}
}

void Character::update(float l_dt) {
	EntityBase::update(l_dt);
	if (m_attackAABB.size.x != 0.0f && m_attackAABB.size.y != 0.0f) {
		updateAttackAABB();
	}

	if (getState() != EntityState::Dying && getState() != EntityState::Attacking && getState() != EntityState::Hurt) {
		if (std::abs(m_velocity.y) >= 0.001f) {
			setState(EntityState::Jumping);
		}
		else if (std::abs(m_velocity.x) >= 0.001f) {
			setState(EntityState::Walking);
		}
		else {
			setState(EntityState::Idle);
		}
	}
	else if (getState() == EntityState::Attacking || getState() == EntityState::Hurt) {
		if (!m_spritesheet.getCurrentAnim()->isPlaying()) {
			setState(EntityState::Idle);
		}
	}
	animate();
	m_spritesheet.update(l_dt);
	if (getState() == EntityState::Dying && !m_spritesheet.getCurrentAnim()->isPlaying()) {
		m_entityManager->remove(m_id);
	}
	if (sf::Sprite* sprite{ m_spritesheet.getSprite() }) {
		const sf::Vector2i spriteSize{ m_spritesheet.getSpriteSize() };
		sprite->setOrigin({ spriteSize.x / 2.0f, static_cast<float>(spriteSize.y) });
	}
	m_spritesheet.setSpritePosition(m_position);
}

void Character::draw(sf::RenderWindow* l_window) {
	m_spritesheet.draw(l_window);
}