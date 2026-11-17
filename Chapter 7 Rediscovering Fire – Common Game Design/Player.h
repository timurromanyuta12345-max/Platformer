#pragma once
#include "Character.h"

struct EventDetails;

class Player : public Character {
public:
	Player(EntityManager* l_manager);
	~Player();

	virtual void onEntityCollision(EntityBase* l_collider, bool l_attack) override;
	void react(EventDetails* l_details);
	virtual size_t getSizeObject() const noexcept override;
};