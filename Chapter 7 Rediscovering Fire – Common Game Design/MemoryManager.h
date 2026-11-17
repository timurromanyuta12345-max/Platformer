#pragma once
#include <memory>
#include <memory_resource>
#include <string_view>

using namespace std;

// Forward declarations to avoid circular include with EventManager.h
struct Binding;
struct PmrDeleter;

class MemoryManager {
private:
	size_t m_requestedBytes;
	std::unique_ptr<void, void(*)(void*)> m_heapBuffer;
	std::pmr::monotonic_buffer_resource m_arena;
	std::pmr::unsynchronized_pool_resource m_pool;
public:
	MemoryManager();
	MemoryManager(size_t bytes);
	MemoryManager(const MemoryManager& l_memoryManager) = delete;
	MemoryManager& operator=(const MemoryManager& l_memoryManager) = delete;
	MemoryManager(MemoryManager&& l_memoryManager) = delete;
	MemoryManager& operator=(MemoryManager&& l_memoryManager) = delete;
	~MemoryManager() = default;

	__forceinline std::pmr::polymorphic_allocator<std::byte> getAlloc() {
		return &m_pool;
	}

	// Deallocate raw memory previously allocated with alloc.allocate(bytes)
	void deallocate(void* ptr, std::size_t bytes);

	std::unique_ptr<Binding, PmrDeleter> allocateCustom(std::string_view l_actionName = std::string_view());
};

class LocalMemoryManager {
private:
	inline static MemoryManager m_manager;
public:
	__forceinline static MemoryManager& getManager() {
		return m_manager;
	}
};