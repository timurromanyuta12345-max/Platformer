#include "SpriteSheet.h"
#include "Anim_Base.h"
#include "MemoryManager.h"
#include "TextureManager.h"
#include "Utilities.h"
#include "Anim_Directional.h"
#include <memory>

SpriteSheet::SpriteSheet(TextureManager* l_textureManager) : m_texture{ LocalMemoryManager::getManager().getAlloc() }, m_sprite{ nullptr }, m_spriteSize{ 0, 0 }, m_spriteScale { 1u, 1u }, m_direction{ Direction::Right }, m_animType{ LocalMemoryManager::getManager().getAlloc() }, m_animationCurrent{ nullptr }, m_textureManager{ l_textureManager } {
	m_animations.max_load_factor(0.5f);
	m_animations.reserve(6uz);
}

SpriteSheet::~SpriteSheet() {
	releaseSheet();
}

void SpriteSheet::releaseSheet() {
	while (m_animations.begin() != m_animations.end()) {
		Anim_Base* animation{ m_animations.begin()->second };
		const size_t bytes{ animation->getSize() };
		std::destroy_at(animation);
		LocalMemoryManager::getManager().deallocate(animation, bytes);
		m_animations.erase(m_animations.begin());
	}
	m_animationCurrent = nullptr;
	if (m_sprite) {
		std::destroy_at(m_sprite);
		LocalMemoryManager::getManager().deallocate(m_sprite, sizeof(sf::Sprite));
		m_sprite = nullptr;
	}
	if (!m_texture.empty()) {
		m_textureManager->releaseResource(m_texture);
		m_texture.clear();
	}
}

void SpriteSheet::setSpriteSize(const sf::Vector2i l_size) {
	m_spriteSize = l_size;
	if (m_sprite) {
		m_sprite->setOrigin({ m_spriteSize.x / 2.0f, static_cast<float>(m_spriteSize.y) });
	}
}

void SpriteSheet::setDirection(const Direction l_direction) {
	if (l_direction == m_direction) [[unlikely]] {
		return;
	}

	m_direction = l_direction;
	if (m_animationCurrent) {
		m_animationCurrent->cropSite();
	}
}

void SpriteSheet::cropSite(const sf::IntRect l_rect) {
	if (m_sprite) {
		m_sprite->setTextureRect(l_rect);
	}
}

bool SpriteSheet::setAnimation(const std::string_view l_name, const bool l_play, const bool l_loop) {
	std::pmr::string strForFind{ l_name, LocalMemoryManager::getManager().getAlloc() };
	auto itr{ m_animations.find(strForFind) };

	if (itr == m_animations.end()) [[unlikely]] {
		return false;
	}

	if (itr->second != m_animationCurrent) {
		if (m_animationCurrent) [[likely]] {
			m_animationCurrent->stop();
		}
		m_animationCurrent = itr->second;
	}

	m_animationCurrent->setLoop(l_loop);
	
	if (l_play) [[likely]] {
		m_animationCurrent->play();
	}

	m_animationCurrent->cropSite();
	return true;
}

void SpriteSheet::update(const float l_dt) {
	if (m_animationCurrent) {
		m_animationCurrent->update(l_dt);
	}
}

void SpriteSheet::draw(sf::RenderWindow* l_window) {
	if (m_sprite) {
		l_window->draw(*m_sprite);
	}
}

bool SpriteSheet::loadSheet(const std::string_view l_file) {
	std::pmr::string tempForIfstream(utils::getWorkingDirectory() + std::pmr::string(l_file, LocalMemoryManager::getManager().getAlloc()));
	std::ifstream sheet(tempForIfstream.c_str());
	if (!sheet) {
		std::println("The file couldn't be opened. Failed loading spritesheet");
		return false;
	}

	releaseSheet();
	std::pmr::string tempForWhile(LocalMemoryManager::getManager().getAlloc());
	std::pmr::string type(LocalMemoryManager::getManager().getAlloc());
	std::pmr::string texture(LocalMemoryManager::getManager().getAlloc());
	std::pmr::string name(LocalMemoryManager::getManager().getAlloc());
	while (std::getline(sheet, tempForWhile)) {
		if (tempForWhile[0] == '|') [[unlikely]] {
			continue;
		}

		std::spanstream stream(tempForWhile);
		stream >> type;

		if (type == "Texture") {
			if (!m_texture.empty()) {
				std::println("Duplicate texture entries in: {}", l_file);
				continue;
			}

			stream >> texture;
			if (!m_textureManager->requireResource(texture)) [[unlikely]] {
				std::println("Could not set up the texture: {}", texture);
				releaseSheet();
				return false;
			}

			m_texture = texture;
			sf::Texture* textureResource{ m_textureManager->getResource(m_texture) };
			if (!textureResource) {
				releaseSheet();
				return false;
			}
			void* rawMemory{ LocalMemoryManager::getManager().getAlloc().allocate(sizeof(sf::Sprite)) };
			m_sprite = std::construct_at(reinterpret_cast<sf::Sprite*>(rawMemory), *textureResource);
		}
		else if (type == "Size") {
			stream >> m_spriteSize.x >> m_spriteSize.y;
			setSpriteSize(m_spriteSize);
		}
		else if (type == "Scale") {
			stream >> m_spriteScale.x >> m_spriteScale.y;
			setSpriteScale(m_spriteScale);
		}
		else if (type == "AnimationType") {
			stream >> m_animType;
		}
		else if (type == "Animation") [[unlikely]] {
			stream >> name;
			if (m_animations.find(name) != m_animations.end()) {
				std::println("Duplicate animation({}) in: {}", name, l_file);
				continue;
			}

			Anim_Base* anim{ nullptr };
			if (m_animType == "Directional") [[likely]] {
				void* rawMemory{ LocalMemoryManager::getManager().getAlloc().allocate(sizeof(Anim_Directional)) };
				anim = std::construct_at(reinterpret_cast<Anim_Directional*>(rawMemory));
			}
			else [[unlikely]] {
				std::println("Unknown animation type: {}", m_animType);
				continue;
			}

			stream >> *anim;
			anim->setSpriteSheet(this);
			anim->setName(name);
			anim->reset();
			m_animations.emplace(name, anim);

			if (m_animationCurrent) [[likely]] {
				continue;
			}

			m_animationCurrent = anim;
			m_animationCurrent->play();
		}
	}

	sheet.close();
	if (!m_sprite || !m_animationCurrent) {
		releaseSheet();
		return false;
	}

	return true;
}