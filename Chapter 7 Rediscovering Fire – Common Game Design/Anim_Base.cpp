#include "Anim_Base.h"
//#include "SpriteSheet.h"
#include "MemoryManager.h"

Anim_Base::Anim_Base() : m_frameCurrent{ 0u }, m_frameStart{ 0u }, m_frameEnd{ 0u }, m_frameRow{ 0u }, m_frameActionStart{ -1 }, m_frameActionEnd{ -1 }, m_frameTime{ 0.0f }, m_elapsedTime{ 0.0f }, m_flags{ false }, m_name { LocalMemoryManager::getManager().getAlloc() }, m_spriteSheet{ nullptr } {}

void Anim_Base::setFrame(const Frame l_frame) {
	if ((l_frame >= m_frameStart && l_frame <= m_frameEnd) || (l_frame >= m_frameEnd && l_frame <= m_frameStart)) [[likely]] {
		m_frameCurrent = l_frame;
	}
}

bool Anim_Base::isInAction() {
	if (m_frameActionStart == -1 || m_frameActionEnd == -1) [[unlikely]] {
		return true;
	}

	return (m_frameCurrent >= m_frameActionStart && m_frameCurrent <= m_frameActionEnd);
}

void Anim_Base::stop() {
	m_flags[static_cast<size_t>(AnimFlags::Playing)] = false;
	reset();
}

void Anim_Base::reset() {
	m_frameCurrent = m_frameStart;
	m_elapsedTime = 0.0f;
	cropSite();
}

void Anim_Base::update(const float l_dt) {
	if (!m_flags[static_cast<size_t>(AnimFlags::Playing)]) [[unlikely]] {
		return;
	}

	if (m_frameTime <= 0.0f) [[unlikely]] {
		return;
	}

	m_elapsedTime += l_dt;
	while (m_elapsedTime >= m_frameTime && m_flags[static_cast<size_t>(AnimFlags::Playing)]) {
		m_elapsedTime -= m_frameTime;
		frameStep();
	}

	cropSite();
}