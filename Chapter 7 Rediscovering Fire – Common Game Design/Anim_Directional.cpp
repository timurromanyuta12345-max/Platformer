#include "Anim_Directional.h"
#include "SpriteSheet.h"

void Anim_Directional::cropSite() {
	sf::IntRect rect({ static_cast<int>(m_spriteSheet->getSpriteSize().x * m_frameCurrent), static_cast<int>(m_spriteSheet->getSpriteSize().y * m_frameRow) }, { m_spriteSheet->getSpriteSize().x, m_spriteSheet->getSpriteSize().y });
	m_spriteSheet->cropSite(rect);

	// Читаем живой указатель на спрайт из твоего спрайт-листа
	sf::Sprite* sprite = m_spriteSheet->getSprite();
	if (sprite) [[likely]] {
		// ВЫСТАВЛЯЕМ ЦЕНТР (Origin) СТРОГО ПО ЦЕНТРУ КАДРА!
		// Это критически важно, чтобы при развороте рыцарь не дергался и не улетал в сторону!
		sprite->setOrigin({ m_spriteSheet->getSpriteSize().x / 2.0f, m_spriteSheet->getSpriteSize().y / 2.0f});

		// АППАРАТНОЕ ЗЕРКАЛИРОВАНИЕ:
		// Проверяем направление из Спрайт-листа. Если лево — ставим минус по X!
		if (m_spriteSheet->getDirection() == Direction::Left) {
			sprite->setScale({ -1.0f, 1.0f }); // Разворачиваем влево!
		}
		else {
			sprite->setScale({ 1.0f, 1.0f });  // Во всех остальных случаях смотрим вправо
		}
	}
}

void Anim_Directional::frameStep() {
	if (m_frameStart < m_frameEnd) [[likely]] {
		++m_frameCurrent;
	}
	else {
		--m_frameCurrent;
	}

	if ((m_frameStart < m_frameEnd && m_frameCurrent > m_frameEnd) || (m_frameStart > m_frameEnd && m_frameCurrent < m_frameEnd)) [[likely]] {
		if (m_flags[static_cast<size_t>(AnimFlags::Loop)]) [[unlikely]] {
			m_frameCurrent = m_frameStart;
			return;
		}

		m_frameCurrent = m_frameEnd;
		pause();
	}
}

void Anim_Directional::readIn(std::spanstream& l_stream) {
	l_stream >> m_frameStart >> m_frameEnd >> m_frameRow >> m_frameTime >> m_frameActionStart >> m_frameActionEnd;
}

size_t Anim_Directional::getSize() const noexcept {
	return sizeof(Anim_Directional);
}