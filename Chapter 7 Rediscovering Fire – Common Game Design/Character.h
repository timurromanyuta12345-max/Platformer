#pragma once
#include "EntityBase.h"
#include "SpriteSheet.h"

class Character : public EntityBase {
private:
	friend class EntityManager;
protected:
	SpriteSheet m_spritesheet;
	float m_jumpVelocity;
	uint32_t m_hitpoints;
	sf::FloatRect m_attackAABB;
	sf::FloatRect m_attackAABBoffset;

	void updateAttackAABB();
	void animate();
public:
	Character(EntityManager* l_manager);
	virtual ~Character() noexcept = default;

	void move(const Direction l_dir);
	void jump();
	void attack();
	void getHurt(const uint32_t l_damage);
	void load(const std::string_view l_path);
	virtual void draw(sf::RenderWindow* l_window) override;
	virtual void update(float l_dt) override;
	virtual void onEntityCollision(EntityBase* l_collider, bool l_attack) override = 0;
};