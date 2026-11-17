#pragma once
#include "Directions.h"
//#include "TextureManager.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <string_view>

class Anim_Base;
class TextureManager;

using Animations = std::pmr::unordered_map<std::pmr::string, Anim_Base*>;

class SpriteSheet {
private:
	std::pmr::string m_texture;
	sf::Sprite* m_sprite;
	sf::Vector2i m_spriteSize;
	sf::Vector2i m_spriteScale;
	Direction m_direction;
	std::pmr::string m_animType;
	Animations m_animations;
	Anim_Base* m_animationCurrent;
	TextureManager* m_textureManager;
public:
	SpriteSheet(TextureManager* l_textureManager);
	~SpriteSheet() noexcept;

	void cropSite(const sf::IntRect l_rect);
	void releaseSheet();
	void update(const float l_dt);
	void draw(sf::RenderWindow* l_window);

	bool loadSheet(const std::string_view l_file);
	bool setAnimation(const std::string_view l_name, const bool l_play = false, const bool l_loop = false);

	__forceinline Anim_Base* getCurrentAnim() {
		return m_animationCurrent;
	}

	// setters and getters
	__forceinline void setTexture(const std::string_view l_texture) {
		m_texture = l_texture;
	}

	__forceinline std::string_view getTexture() {
		return m_texture;
	}

	__forceinline void setSpritePosition(sf::Vector2f l_position) {
		if (m_sprite) {
			m_sprite->setPosition(l_position);
		}
	}

	__forceinline sf::Sprite* getSprite() {
		return m_sprite;
	}

	void setSpriteSize(const sf::Vector2i l_size);

	__forceinline sf::Vector2i getSpriteSize() {
		return m_spriteSize;
	}

	__forceinline void setSpriteScale(const sf::Vector2i l_scale) {
		m_spriteScale = l_scale;
	}

	__forceinline sf::Vector2i getSpriteScale() {
		return m_spriteScale;
	}

	void setDirection(const Direction l_direction);

	__forceinline Direction getDirection() {
		return m_direction;
	}

	__forceinline void setAnimType(const std::string_view l_animType) {
		m_animType.assign(l_animType);
	}

	__forceinline std::string_view getAnimType() {
		return m_animType;
	}

	__forceinline Animations& getAnimations() {
		return m_animations;
	}

	__forceinline void setAnimCurrent(Anim_Base* l_animBase) {
		m_animationCurrent = l_animBase;
	}

	__forceinline TextureManager* getTextureManager() {
		return m_textureManager;
	}
};