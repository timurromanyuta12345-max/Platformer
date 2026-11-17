#pragma once
#include <SFML/Graphics.hpp>
#include "ResourceManager.h"
#include <memory>

class TextureManager : public ResourceManager<TextureManager, sf::Texture> {
public:
	TextureManager() : ResourceManager("textures.cfg") {}

	sf::Texture* load(const std::string_view l_path) {
		void* rawMemory{ LocalMemoryManager::getManager().getAlloc().allocate(sizeof(sf::Texture)) };
		sf::Texture* texture{ std::construct_at(reinterpret_cast<sf::Texture*>(rawMemory)) };
		std::pmr::string tempForIf{ l_path, LocalMemoryManager::getManager().getAlloc() };
		if (!texture->loadFromFile(utils::getWorkingDirectory() + tempForIf)) [[unlikely]] {
			std::destroy_at(texture);
			LocalMemoryManager::getManager().deallocate(rawMemory, sizeof(sf::Texture));
			texture = nullptr;
			std::println("! Failed to load texture: {}", l_path);
		}

		return texture;
	}
};