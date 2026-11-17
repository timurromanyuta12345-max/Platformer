#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>
#include <bitset>
#include <vector>

enum class EntityType : uint8_t { Base, Enemy, Player, Count };
enum class EntityState : uint8_t { Idle, Walking, Jumping, Attacking, Hurt, Dying, Count };
enum class Axis_Collisions : uint8_t { CollidingOnX, CollidingOnY, CollidingOnGround, Count };

struct TileInfo;

struct CollisionElement {
	float m_area;
	TileInfo* m_tile;
	sf::FloatRect m_tileBounds;

	CollisionElement(float l_area, TileInfo* l_info, const sf::FloatRect& l_bounds) : m_area{ l_area }, m_tile{ l_info }, m_tileBounds{ l_bounds } {}
	static bool sortCollisions(const CollisionElement& l_1, const CollisionElement& l_2) {
		return l_1.m_area > l_2.m_area;
	}
};

using Collisions = std::pmr::vector<CollisionElement>;

class EntityManager;
class EntityBase {
private:
	friend class EntityManager;
protected:
	std::pmr::string m_name;
	EntityType m_type;
	uint32_t m_id;
	sf::Vector2f m_position;
	sf::Vector2f m_positionOld;
	sf::Vector2f m_velocity;
	sf::Vector2f m_maxVelocity;
	sf::Vector2f m_speed;
	sf::Vector2f m_acceleration;
	sf::Vector2f m_friction;
	TileInfo* m_referenceTile;
	sf::Vector2f m_size;
	sf::FloatRect m_AABB;
	EntityState m_state;
	std::bitset<static_cast<size_t>(Axis_Collisions::Count)> m_flags;
	Collisions m_collisions;
	EntityManager* m_entityManager;

	void updateAABB();
	void checkCollisions();
	void resolveCollisons();
	virtual void onEntityCollision(EntityBase* l_collider, bool l_attack) = 0;
public:
	EntityBase(EntityManager* l_entityManager);
	virtual ~EntityBase() noexcept = default;

	void move(float l_x, float l_y);
	void addVelocity(float l_x, float l_y);
	void accelerate(float l_x, float l_y);
	//void setAcceleration(float l_x, float l_y);
	void applyFriction(float l_x, float l_y);
	virtual void update(float l_dt);
	virtual void draw(sf::RenderWindow* l_window) = 0;
	virtual size_t getSizeObject() const noexcept = 0;

	// Getters & setters
	
	__forceinline std::pmr::string& getName() {
		return m_name;
	}

	__forceinline void setName(std::pmr::string& l_string) {
		m_name.assign(l_string);
	}

	__forceinline EntityType getType() {
		return m_type;
	}

	__forceinline void setType(EntityType l_type) {
		m_type = l_type;
	}

	__forceinline uint32_t getID() {
		return m_id;
	}

	__forceinline void setID(uint32_t l_id) {
		m_id = l_id;
	}

	__forceinline sf::Vector2f getPosition() {
		return m_position;
	}

	void setPosition(float l_x, float l_y);
	void setPosition(sf::Vector2f l_position);

	__forceinline sf::Vector2f getPositionOld() {
		return m_positionOld;
	}

	__forceinline void setPositionOld(sf::Vector2f l_positionOld) {
		m_positionOld = l_positionOld;
	}

	__forceinline sf::Vector2f getVelocity() {
		return m_velocity;
	}

	__forceinline void setVelocity(sf::Vector2f l_velocity) {
		m_velocity = l_velocity;
	}

	__forceinline sf::Vector2f getMaxVelocity() {
		return m_maxVelocity;
	}

	__forceinline void setMaxVelocity(sf::Vector2f l_maxVelocity) {
		m_maxVelocity = l_maxVelocity;
	}

	__forceinline sf::Vector2f getSpeed() {
		return m_speed;
	}

	__forceinline void setSpeed(sf::Vector2f l_speed) {
		m_speed = l_speed;
	}

	__forceinline sf::Vector2f getAcceleration() {
		return m_acceleration;
	}

	__forceinline void setAcceleration(sf::Vector2f l_acceleration) {
		m_acceleration = l_acceleration;
	}

	__forceinline sf::Vector2f getFriction() {
		return m_friction;
	}

	__forceinline void setFriction(sf::Vector2f l_friction) {
		m_friction = l_friction;
	}

	__forceinline TileInfo* getReferenceTile() {
		return m_referenceTile;
	}

	__forceinline void setReferenceTile(TileInfo* l_tileInfo) {
		m_referenceTile = l_tileInfo;
	}

	__forceinline sf::Vector2f getSize() {
		return m_size;
	}

	void setSize(sf::Vector2f l_size);

	__forceinline sf::FloatRect getAABB() {
		return m_AABB;
	}

	__forceinline void setAABB(sf::FloatRect l_AABB) {
		m_AABB = l_AABB;
	}

	__forceinline EntityState getState() {
		return m_state;
	}

	void setState(EntityState l_EntityState);

	__forceinline std::bitset<static_cast<size_t>(Axis_Collisions::Count)> getFlags() {
		return m_flags;
	}

	__forceinline Collisions& getCollisions() {
		return m_collisions;
	}

	__forceinline EntityManager* getEntityManager() {
		return m_entityManager;
	}
};