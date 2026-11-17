#include "MemoryManager.h"
#include <bit>
#include "EventManager.h"
#include <memory>
//#include "DebugAllocTracker.h"

using namespace std;

MemoryManager::MemoryManager() : m_requestedBytes{ bit_ceil(1024uz * 1024uz) }, m_heapBuffer{ _aligned_malloc(m_requestedBytes, 64uz), _aligned_free }, m_arena{ m_heapBuffer.get(), m_requestedBytes, pmr::new_delete_resource() }, m_pool{ &m_arena } {}

MemoryManager::MemoryManager(size_t bytes) : m_requestedBytes{ bit_ceil(bytes) }, m_heapBuffer{ _aligned_malloc(m_requestedBytes, 64uz), _aligned_free }, m_arena{ m_heapBuffer.get(), m_requestedBytes, pmr::new_delete_resource() }, m_pool{ &m_arena } {}

PmrBindingPtr MemoryManager::allocateCustom(std::string_view l_actionName) {
	auto alloc = LocalMemoryManager::getManager().getAlloc();
	void* rawMemory = alloc.allocate(sizeof(Binding));
	Binding* bindingAdress = std::construct_at(reinterpret_cast<Binding*>(rawMemory), l_actionName, alloc);
	PmrBindingPtr binding(bindingAdress, PmrDeleter(alloc));
	return binding;
}

void MemoryManager::deallocate(void* ptr, std::size_t bytes) {
	if (!ptr) {
		return;
	}

	auto alloc = getAlloc();
	alloc.deallocate(reinterpret_cast<std::byte*>(ptr), bytes);
}