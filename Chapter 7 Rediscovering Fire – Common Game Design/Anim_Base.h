#pragma once
#include <cstdint>
#include <string_view>
#include <spanstream>
#include <bitset>

class SpriteSheet;
using Frame = uint32_t;

enum class AnimFlags : uint8_t { Loop, Playing, Count };

class Anim_Base {
protected:
	Frame m_frameCurrent;
	Frame m_frameStart;
	Frame m_frameEnd;
	Frame m_frameRow;
	int m_frameActionStart;
	int m_frameActionEnd;
	float m_frameTime;
	float m_elapsedTime;
	std::bitset<static_cast<size_t>(AnimFlags::Count)> m_flags;
	std::pmr::string m_name;
	SpriteSheet* m_spriteSheet;

	virtual void frameStep() = 0;
	virtual void cropSite() = 0;
	virtual void readIn(std::spanstream& l_stream) = 0;
	virtual size_t getSize() const noexcept = 0;
private:
	friend class SpriteSheet;
public:
	Anim_Base();
	virtual ~Anim_Base() noexcept = default;

	// Setters and getters
	void setFrame(const Frame l_frame);

	__forceinline Frame getFrameCurrent() {
		return m_frameCurrent;
	}

	__forceinline Frame getFrameStart() {
		return m_frameStart;
	}

	__forceinline Frame getFrameEnd() {
		return m_frameEnd;
	}

	__forceinline void setFrameRow(Frame l_frameRow) {
		m_frameRow = l_frameRow;
	}

	__forceinline Frame getFrameRow() {
		return m_frameRow;
	}

	bool isInAction();

	__forceinline bool isPlaying() {
		return m_flags[static_cast<size_t>(AnimFlags::Playing)];
	}

	__forceinline void setFrameActionStart(int l_frameActionStart) {
		l_frameActionStart = l_frameActionStart;
	}

	__forceinline int getFrameActionStart() {
		return m_frameActionStart;
	}

	__forceinline void setFrameActionEnd(int l_frameActionEnd) {
		m_frameActionEnd = l_frameActionEnd;
	}

	__forceinline int getFrameActionEnd() {
		return m_frameActionEnd;
	}

	__forceinline void setFrameTime(float l_frameTime) {
		m_frameTime = l_frameTime;
	}

	__forceinline float getFrameTime() {
		return m_frameTime;
	}

	__forceinline void setElpasedTime(float l_elapsedTime) {
		m_elapsedTime = l_elapsedTime;
	}

	__forceinline float getElpasedTime() {
		return m_elapsedTime;
	}

	__forceinline void setLoop(bool l_loop) {
		m_flags[static_cast<size_t>(AnimFlags::Loop)] = l_loop;
	}

	__forceinline bool getLoop() {
		return m_flags[static_cast<size_t>(AnimFlags::Loop)];
	}

	__forceinline void setPlaying(bool l_playing) {
		m_flags[static_cast<size_t>(AnimFlags::Playing)] = l_playing;
	}

	__forceinline bool getPlaying() {
		return m_flags[static_cast<size_t>(AnimFlags::Playing)];
	}

	__forceinline void setName(std::string_view l_name) {
		m_name.assign(l_name);
	}

	__forceinline std::string_view getName() {
		return m_name;
	}

	__forceinline void setSpriteSheet(SpriteSheet* l_spriteSheet) {
		m_spriteSheet = l_spriteSheet;
	}

	__forceinline SpriteSheet* getSpriteSheet() {
		return m_spriteSheet;
	}

	__forceinline void play() {
		m_flags[static_cast<size_t>(AnimFlags::Playing)] = true;
	}

	__forceinline void pause() {
		m_flags[static_cast<size_t>(AnimFlags::Playing)] = false;
	}

	void stop();
	void reset();

	virtual void update(const float l_dt);

	friend std::spanstream& operator>>(std::spanstream& l_stream, Anim_Base& ab) {
		ab.readIn(l_stream);
		return l_stream;
	}
};