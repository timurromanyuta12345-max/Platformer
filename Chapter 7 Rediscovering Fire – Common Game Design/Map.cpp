#include "Map.h"
#include "StateManager.h"
#include "SharedContext.h"
#include "BaseState.h"
#include "EntityManager.h"
#include "Window.h"
#include "EntityBase.h"
#include <cmath>
#include <fstream>
#include <print>
#include <spanstream>
#include <memory>

Map::Map(SharedContext* l_sharedContext, BaseState* l_currentState) : m_tileSet{ LocalMemoryManager::getManager().getAlloc() }, m_tileMap{ LocalMemoryManager::getManager().getAlloc() }, m_enemySpawns{ LocalMemoryManager::getManager().getAlloc() }, m_background{ nullptr }, m_defaultTile{ l_sharedContext }, m_maxMapSize{ 96u, 54u }, m_tileCount{ 0u }, m_tileSetCount{ 0u }, m_mapGravity{ 512.0f }, m_nextMap{ LocalMemoryManager::getManager().getAlloc() }, m_loadNextMap{ false }, m_backgroundTexture{ LocalMemoryManager::getManager().getAlloc() }, m_currentState { l_currentState }, m_sharedContext{ l_sharedContext } {
	m_sharedContext->m_gameMap = this;
	loadTiles("Tiles.cfg");
}

Map::~Map() {
	purgeMap();
	purgeTileSet();
	m_sharedContext->m_gameMap = nullptr;
}

Tile* Map::getTile(uint32_t l_x, uint32_t l_y) {
	auto itr{ m_tileMap.find(convertCoords(l_x, l_y)) };
	return (itr != m_tileMap.end() ? itr->second : nullptr);
}

uint32_t Map::convertCoords(const uint32_t l_x, const uint32_t l_y) {
	return (l_y * m_maxMapSize.x) + l_x;
}

void Map::update(float l_dt) {
	if (m_loadNextMap) [[unlikely]] {
		m_loadNextMap = false;
		if (m_nextMap != "") [[likely]] {
			std::pmr::string nextMap{ std::move(m_nextMap), LocalMemoryManager::getManager().getAlloc() };
			m_nextMap = "";
			loadMap(nextMap);
		}
		else {
			m_currentState->getStateManager()->switchTo(BaseState::StateType::GameOver);
		}
	}
	for (auto& enemy : m_enemySpawns) {
		if (!enemy.respawnPending) {
			if (!m_sharedContext->m_entityManager->find(enemy.entityId)) {
				enemy.respawnPending = true;
				enemy.respawnTimer = 3.0f;
			}
			continue;
		}

		enemy.respawnTimer -= l_dt;
		if (enemy.respawnTimer <= 0.0f) {
			const int entityId{ m_sharedContext->m_entityManager->add(EntityType::Enemy, enemy.type) };
			if (entityId >= 0) {
				EntityBase* respawnedEnemy{ m_sharedContext->m_entityManager->find(static_cast<uint32_t>(entityId)) };
				if (respawnedEnemy) {
					respawnedEnemy->setPosition(enemy.position);
					enemy.entityId = static_cast<uint32_t>(entityId);
					enemy.respawnPending = false;
				}
			}
		}
	}

	sf::FloatRect viewSpace{ m_sharedContext->m_window->getViewSpace() };
	if (m_background) {
		m_background->setPosition(viewSpace.position);
	}
}

void Map::draw() {
	sf::RenderWindow* l_window{ m_sharedContext->m_window->getRenderWindow() };
	if (!m_backgroundTexture.empty()) {
		l_window->draw(*m_background);
	}
	sf::FloatRect viewSpace{ m_sharedContext->m_window->getViewSpace() };

	sf::Vector2i tileBegin( std::floor(viewSpace.position.x / Sheet::Tile_Size), std::floor(viewSpace.position.y / Sheet::Tile_Size));
	sf::Vector2i tileEnd(std::ceil((viewSpace.position.x + viewSpace.size.x) / Sheet::Tile_Size), std::ceil((viewSpace.position.y + viewSpace.size.y) / Sheet::Tile_Size));

	for (int x{ tileBegin.x }; x <= tileEnd.x; ++x) {
		for (int y{ tileBegin.y }; y <= tileEnd.y; ++y) {
			if (x < 0u || y < 0u) [[unlikely]] {
				continue;
			}

			Tile* tile{ getTile(x, y) };
			if (!tile || !tile->m_properties || !tile->m_properties->m_sprite) [[unlikely]] {
				continue;
			}

			sf::Sprite& sprite{ *tile->m_properties->m_sprite };
			sprite.setPosition(sf::Vector2f(x * Sheet::Tile_Size, y * Sheet::Tile_Size));
			l_window->draw(sprite);
			++m_tileCount;
		}
	}
}

void Map::purgeMap() {
	m_tileCount = 0u;
	for (auto& itr : m_tileMap) {
		std::destroy_at(itr.second);
		LocalMemoryManager::getManager().deallocate(itr.second, sizeof(Tile));
	}

	m_tileMap.clear();
	m_sharedContext->m_entityManager->purge();
	m_enemySpawns.clear();
	if (m_background) {
		std::destroy_at(m_background);
		LocalMemoryManager::getManager().deallocate(m_background, sizeof(sf::Sprite));
		m_background = nullptr;
	}

	if (m_backgroundTexture == "") [[unlikely]] {
		return;
	}

	m_sharedContext->m_textureManager->releaseResource(m_backgroundTexture);
	m_backgroundTexture.assign("");
}

void Map::purgeTileSet() {
	for (auto& itr : m_tileSet) {
		std::destroy_at(itr.second);
		LocalMemoryManager::getManager().deallocate(itr.second, sizeof(TileInfo));
	}
	
	m_tileSet.clear();
	m_tileSetCount = 0u;
}

void Map::loadTiles(const std::string_view l_path) {
	std::pmr::string tempForIfstream{ utils::getWorkingDirectory() + std::pmr::string(l_path, LocalMemoryManager::getManager().getAlloc()) };
	std::ifstream file{ tempForIfstream.c_str() };
	if (!file) {
		std::println("! Failed loading tile set file:{}", l_path);
		return;
	}

	std::pmr::string line{ LocalMemoryManager::getManager().getAlloc() };
	int tileID;
	while (std::getline(file, line)) {
		if (line[0] == '|') [[unlikely]] {
			continue;
		}

		std::spanstream keystream{ line };
		keystream >> tileID;
		if (tileID < 0) [[unlikely]] {
			continue;
		}

		void* rawMemory{ LocalMemoryManager::getManager().getAlloc().allocate(sizeof(TileInfo)) };
		TileInfo* tile{ std::construct_at(reinterpret_cast<TileInfo*>(rawMemory), m_sharedContext, "TileSheet", tileID) };
		if (!tile->m_sprite) [[unlikely]] {
			std::destroy_at(tile);
			LocalMemoryManager::getManager().deallocate(tile, sizeof(TileInfo));
			continue;
		}

		keystream >> tile->m_name >> tile->m_friction.x >> tile->m_friction.y >> tile->m_deadly;
		if (!m_tileSet.emplace(tileID, tile).second) [[unlikely]] {
			std::println("! Duplicate tile type:{}", tile->m_name);
			std::destroy_at(tile);
			LocalMemoryManager::getManager().deallocate(tile, sizeof(TileInfo));
		}
	}

	file.close();
}

void Map::loadMap(const std::string_view l_path) {
	std::pmr::string tempForIfstream(utils::getWorkingDirectory() + std::pmr::string(l_path, LocalMemoryManager::getManager().getAlloc()));
	std::ifstream file{ tempForIfstream.c_str() };
	if (!file) {
		std::println("! The file couldn't be opened. Failed loading map");
		return;
	}

	purgeMap();
	std::pmr::string tempForWhile(LocalMemoryManager::getManager().getAlloc());
	std::pmr::string type(LocalMemoryManager::getManager().getAlloc());
	std::pmr::string warp(LocalMemoryManager::getManager().getAlloc());
	std::pmr::string enemyName(LocalMemoryManager::getManager().getAlloc());
	int tileID;
	sf::Vector2i tileCoords;
	sf::Vector2f scaleFactors;
	while (std::getline(file, tempForWhile)) {
		if (tempForWhile[0] == '|') [[unlikely]] {
			continue;
		}

		std::spanstream keystream(tempForWhile);
		keystream >> type;

		if (type == "TILE") [[likely]] {
			tileID = 0;
			keystream >> tileID;
			if (tileID < 0) [[unlikely]] {
				std::println("! Bad tile id:{}", tileID);
				continue;
			}

			auto itr{ m_tileSet.find(tileID) };
			if (itr == m_tileSet.end()) [[unlikely]] {
				std::println("! Tile ID({}) wasn't found in tileset", tileID);
				continue;
			}

			keystream >> tileCoords.x >> tileCoords.y;
			if (tileCoords.x < 0 || tileCoords.y < 0 || tileCoords.x >= m_maxMapSize.x || tileCoords.y >= m_maxMapSize.y) [[unlikely]] {
				std::println("! Tile is out of range:{}  {}", tileCoords.x, tileCoords.y);
				continue;
			}

			void* rawMemory{ LocalMemoryManager::getManager().getAlloc().allocate(sizeof(Tile)) };
			Tile* tile{ std::construct_at(reinterpret_cast<Tile*>(rawMemory)) };
			tile->m_properties = itr->second;
			if (!m_tileMap.emplace(convertCoords(tileCoords.x, tileCoords.y), tile).second) [[unlikely]] {
				std::println("! Duplicate tile! : {}  {}", tileCoords.x, tileCoords.y);
				std::destroy_at(tile);
				LocalMemoryManager::getManager().deallocate(tile, sizeof(Tile));
				tile = nullptr;
				tileCoords = { 0, 0 };
				continue;
			}

			keystream >> warp;
			tile->m_warp = false;
			if (warp == "WARP") [[unlikely]] {
				tile->m_warp = true;
			}
		}
		else if (type == "BACKGROUND") [[unlikely]] {
			if (m_backgroundTexture != "") [[likely]] {
				continue;
			}

			keystream >> m_backgroundTexture;
			if (!m_sharedContext->m_textureManager->requireResource(m_backgroundTexture)) [[unlikely]] {
				m_backgroundTexture = "";
				continue;
			}

			sf::Texture* texture{ m_sharedContext->m_textureManager->getResource(m_backgroundTexture) };
			void* rawMemory{ LocalMemoryManager::getManager().getAlloc().allocate(sizeof(sf::Sprite)) };
			m_background = std::construct_at(reinterpret_cast<sf::Sprite*>(rawMemory), *texture);
			sf::Vector2f viewSize{ m_currentState->getView().getSize() };
			sf::Vector2u textureSize{ texture->getSize() };
			scaleFactors.x = viewSize.x / textureSize.x;
			scaleFactors.y = viewSize.y / textureSize.y; 
			m_background->setScale(scaleFactors);
			scaleFactors = { 0.0f, 0.0f };
		}
		else if (type == "SIZE") [[unlikely]] {
			keystream >> m_maxMapSize.x >> m_maxMapSize.y;
		}
		else if (type == "GRAVITY") [[unlikely]] {
			keystream >> m_mapGravity;
		}
		else if (type == "DEFAULT_FRICTION") [[unlikely]] {
			keystream >> m_defaultTile.m_friction.x >> m_defaultTile.m_friction.y;
		}
		else if (type == "NEXTMAP") [[unlikely]] {
			keystream >> m_nextMap;
		}
		else if (type == "PLAYER") [[unlikely]] {
			int playerID{ m_sharedContext->m_entityManager->add(EntityType::Player) };

			if (playerID < 0) [[unlikely]] {
				continue;
			}

			float playerX = 0.0f, playerY = 0.0f;
			keystream >> playerX >> playerY;
			const float tileSize{ static_cast<float>(Sheet::Tile_Size) };
			const sf::Vector2f playerPosition{ (playerX + 0.5f) * tileSize, playerY * tileSize };
			m_sharedContext->m_entityManager->find(playerID)->setPosition(playerPosition);
			m_playerStart = sf::Vector2u{ static_cast<uint32_t>(playerPosition.x), static_cast<uint32_t>(playerPosition.y) };
		}
		else if (type == "ENEMY") [[unlikely]] {
			keystream >> enemyName;
			int enemyID{ m_sharedContext->m_entityManager->add(EntityType::Enemy, enemyName) };

			if (enemyID < 0) [[unlikely]] {
				continue;
			}

			float enemyX = 0.0f, enemyY = 0.0f;
			keystream >> enemyX >> enemyY;
			const float tileSize{ static_cast<float>(Sheet::Tile_Size) };
			const sf::Vector2f enemyPosition{ (enemyX + 0.5f) * tileSize, enemyY * tileSize };
			m_sharedContext->m_entityManager->find(enemyID)->setPosition(enemyPosition);
			m_enemySpawns.emplace_back(enemyName, enemyPosition, static_cast<uint32_t>(enemyID), LocalMemoryManager::getManager().getAlloc());
		}
	}
}