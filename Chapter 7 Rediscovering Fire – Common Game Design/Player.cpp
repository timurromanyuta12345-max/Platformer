#include "Player.h"
#include "EventManager.h"
#include "EntityManager.h"
#include "BaseState.h"
#include "Anim_Base.h"
#include "SharedContext.h"

Player::Player(EntityManager* l_manager) : Character(l_manager) {
	load("Player.char");
	m_type = EntityType::Player;

	EventManager* events{ m_entityManager->getContext()->m_eventManager };
	events->addCallback<Player>(BaseState::StateType::Game, "Player_MoveLeft", &Player::react, this);
	events->addCallback<Player>(BaseState::StateType::Game, "Player_MoveRight", &Player::react, this);
	events->addCallback<Player>(BaseState::StateType::Game, "Player_Jump", &Player::react, this);
	events->addCallback<Player>(BaseState::StateType::Game, "Player_Attack", &Player::react, this);
}

Player::~Player() {
	EventManager* events{ m_entityManager->getContext()->m_eventManager };
	events->removeCallback(BaseState::StateType::Game, "Player_MoveLeft");
	events->removeCallback(BaseState::StateType::Game, "Player_MoveRight");
	events->removeCallback(BaseState::StateType::Game, "Player_Jump");
	events->removeCallback(BaseState::StateType::Game, "Player_Attack");
}

void Player::onEntityCollision(EntityBase* l_collider, bool l_attack) {
	if (m_state == EntityState::Dying) [[unlikely]] {
		return;
	}

	if (l_attack) {
		if (m_state != EntityState::Attacking) {
			return;
		}

		if (!m_spritesheet.getCurrentAnim()->isInAction()) {
			return;
		}

		if (l_collider->getType() != EntityType::Enemy && l_collider->getType() != EntityType::Player) {
			return;
		}

		Character* opponent{ static_cast<Character*>(l_collider) };
		opponent->getHurt(1u);

		if (m_position.x > opponent->getPosition().x) {
			opponent->addVelocity(-32.0f, 0.0f);
		}
		else {
			opponent->addVelocity(32.0f, 0.0f);
		}
	}
	else {
		// Other behavior
	}
}

void Player::react(EventDetails* l_details) {
	if (l_details->m_name == "Player_MoveLeft") {
		move(Direction::Left);
	}
	else if (l_details->m_name == "Player_MoveRight") {
		move(Direction::Right);
	}
	else if (l_details->m_name == "Player_Jump") {
		jump();
	}
	else if (l_details->m_name == "Player_Attack") {
		attack();
	}
}

size_t Player::getSizeObject() const noexcept {
	return sizeof(Player);
}