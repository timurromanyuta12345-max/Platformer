#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "BaseState.h"
#include "SharedContext.h"
#include "TextureManager.h"

enum Sheet : uint8_t { Tile_Size = 20u, Sheet_Width = 180u, Sheet_Height = 20u };
using TileID = unsigned int;

struct TileInfo {
	sf::Sprite* m_sprite;
	TileID m_id;
	std::pmr::string m_name;
	sf::Vector2f m_friction;
	bool m_deadly;
	SharedContext* m_sharedContext;
	std::pmr::string m_texture;

	TileInfo(SharedContext* l_context, const std::string_view l_texture = "", TileID l_id = 0u) : m_sprite{ nullptr }, m_id{ 0u }, m_name{ LocalMemoryManager::getManager().getAlloc() }, m_deadly{ false }, m_sharedContext{ l_context }, m_texture{ LocalMemoryManager::getManager().getAlloc() } {
		TextureManager* textureManager{ l_context->m_textureManager };
		if (l_texture == "") [[unlikely]] {
			return;
		}

		if (!textureManager->requireResource(l_texture)) [[unlikely]] {
			return;
		}

		m_texture.assign(l_texture);
		m_id = l_id;
		void* rawMemory{ LocalMemoryManager::getManager().getAlloc().allocate(sizeof(sf::Sprite)) };
		m_sprite = std::construct_at(reinterpret_cast<sf::Sprite*>(rawMemory), *textureManager->getResource(m_texture));
		sf::IntRect tileBoundaries{ sf::Vector2i(m_id % (Sheet::Sheet_Width / Sheet::Tile_Size) * Sheet::Tile_Size, m_id / (Sheet::Sheet_Width / Sheet::Tile_Size) * Sheet::Tile_Size), sf::Vector2i(Sheet::Tile_Size, Sheet::Tile_Size) };
		m_sprite->setTextureRect(tileBoundaries);
	}

	~TileInfo() {
		if (m_sprite) {
			std::destroy_at(m_sprite);
			LocalMemoryManager::getManager().getAlloc().deallocate(reinterpret_cast<std::byte*>(m_sprite), sizeof(sf::Sprite));
			m_sprite = nullptr;
		}

		if (m_texture == "") [[unlikely]] {
			return;
		}

		m_sharedContext->m_textureManager->releaseResource(m_texture);
	}
};

struct Tile {
	TileInfo* m_properties;
	bool m_warp;
};

using TileMap = std::pmr::unordered_map<TileID, Tile*>;
using TileSet = std::pmr::unordered_map<TileID, TileInfo*>;

class Map {
private:
	struct EnemySpawn {
		std::pmr::string type;
		sf::Vector2f position;
		uint32_t entityId;
		float respawnTimer;
		bool respawnPending;

		EnemySpawn(std::string_view l_type, sf::Vector2f l_position, uint32_t l_entityId, std::pmr::polymorphic_allocator<std::byte> l_alloc)
			: type{ l_type, l_alloc }, position{ l_position }, entityId{ l_entityId }, respawnTimer{ 0.0f }, respawnPending{ false } {}
	};

	TileSet m_tileSet;
	TileMap m_tileMap;
	std::pmr::vector<EnemySpawn> m_enemySpawns;
	sf::Sprite* m_background;
	TileInfo m_defaultTile;
	sf::Vector2u m_maxMapSize;
	sf::Vector2u m_playerStart;
	uint32_t m_tileCount;
	uint32_t m_tileSetCount;
	float m_mapGravity;
	std::pmr::string m_nextMap;
	bool m_loadNextMap;
	std::pmr::string m_backgroundTexture;
	BaseState* m_currentState;
	SharedContext* m_sharedContext;

	uint32_t convertCoords(const uint32_t l_x, const uint32_t l_y);

	void loadTiles(const std::string_view l_path);
	void purgeMap();
	void purgeTileSet();
public:
	Map(SharedContext* l_sharedContext, BaseState* l_currentState);
	~Map();

	Tile* getTile(uint32_t l_x, uint32_t l_y);

	__forceinline TileInfo* getDefaultTile() {
		return &m_defaultTile;
	}

	__forceinline float getGravity() const {
		return m_mapGravity;
	}

	__forceinline uint32_t getTileSize() const {
		return static_cast<uint32_t>(Sheet::Tile_Size);
	}

	__forceinline const sf::Vector2u getMapSize() const {
		return m_maxMapSize;
	}

	__forceinline const sf::Vector2u getPlayerStart() const {
		return m_playerStart;
	}

	void loadMap(const std::string_view l_path);

	__forceinline void loadNext() {
		m_loadNextMap = true;
	}

	void update(float l_dt);
	void draw();
};