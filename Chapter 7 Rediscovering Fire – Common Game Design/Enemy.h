#pragma once
#include "Character.h"
#include <random>

struct Random {
	std::random_device m_rd;
	std::seed_seq m_seeds;
	std::mt19937 m_mt;

	Random() : m_rd{}, m_seeds{ m_rd(), m_rd(), m_rd(), m_rd(), m_rd(), m_rd(), m_rd(), m_rd() }, m_mt{ m_seeds } {}
	Random(const Random& l_random) = delete;
	Random& operator=(const Random& l_random) = delete;
	Random(Random&& l_random) = delete;
	Random& operator=(Random&& l_random) = delete;
	~Random() = default;

	std::uniform_int_distribution<int> go_or_stay{ 1, 120 };
	std::uniform_int_distribution<int> distance{ 0, 65 };
	std::uniform_int_distribution<int> direction{ 0, 1 };
	std::uniform_int_distribution<int> chase{ 0, 2 };
};

class Enemy : public Character {
private:
	sf::Vector2f m_destination;
	bool m_hasDestination;
	bool m_chaseForPlayer;
	static Random engine;
public:
	Enemy(EntityManager* l_manager);
	~Enemy() = default;

	virtual void onEntityCollision(EntityBase* l_collider, bool l_attack) override;
	virtual void update(float l_dt) override;
	virtual size_t getSizeObject() const noexcept override;
};