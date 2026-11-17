#pragma once
#include "MemoryManager.h"
#include "Utilities.h"
#include <unordered_map>
#include <string_view>
#include <utility>
#include <cstdint>
#include <fstream>
#include <spanstream>
#include <print>

template<typename Derived, typename T>
class ResourceManager {
private:
	std::pmr::unordered_map<std::pmr::string, std::pair<T*, uint32_t>> m_resources;
	std::pmr::unordered_map<std::pmr::string, std::pmr::string> m_paths;

	std::pair<T*, uint32_t>* find(const std::string_view l_id) {
		std::pmr::string tempId(l_id, LocalMemoryManager::getManager().getAlloc());
		auto itr{ m_resources.find(tempId) };
		return (itr != m_resources.end() ? &itr->second : nullptr);
	}

	bool unload(const std::string_view l_id) {
		std::pmr::string tempId(l_id, LocalMemoryManager::getManager().getAlloc());
		auto itr{ m_resources.find(tempId) };
		if (itr == m_resources.end()) [[unlikely]] {
			return false;
		}

		std::destroy_at(itr->second.first);
		LocalMemoryManager::getManager().deallocate(itr->second.first, sizeof(T));
		m_resources.erase(itr);
		return true;
	}


	void loadPaths(const std::string_view l_pathFile) {
		std::pmr::string tempForIfstream(utils::getWorkingDirectory() + std::pmr::string(l_pathFile, LocalMemoryManager::getManager().getAlloc()));
		//std::pmr::string tempForPaths(l_pathFile, LocalMemoryManager::getManager().getAlloc());
		std::ifstream paths(tempForIfstream.c_str());
		if (paths.is_open()) [[likely]] {
			std::pmr::string line{ LocalMemoryManager::getManager().getAlloc() }, pathName{ LocalMemoryManager::getManager().getAlloc() }, path{ LocalMemoryManager::getManager().getAlloc() };
			while (std::getline(paths, line)) {
				std::spanstream spanStream{ line };
				spanStream >> pathName;
				spanStream >> path;
				m_paths.emplace(pathName, path);
			}


			paths.close();
			return;
		}

		std::println("! Failed loading the path file: {}", l_pathFile);
	}
public:
	ResourceManager(const std::string_view l_pathFile) : m_resources{ LocalMemoryManager::getManager().getAlloc() }, m_paths{ LocalMemoryManager::getManager().getAlloc() } {
		m_resources.max_load_factor(0.5f);
		m_paths.max_load_factor(0.5f);
		m_resources.reserve(7uz);
		m_paths.reserve(7uz);
		loadPaths(l_pathFile);
	}
	virtual ~ResourceManager() noexcept {
		purgeResources();
	}

	void purgeResources() {
		m_paths.clear();

		while (m_resources.begin() != m_resources.end()) {
			std::destroy_at(m_resources.begin()->second.first);
			LocalMemoryManager::getManager().deallocate(m_resources.begin()->second.first, sizeof(T));
			m_resources.erase(m_resources.begin());
		}
	}

	T* getResource(const std::string_view l_id) {
		auto res{ find(l_id) };
		return (res ? res->first : nullptr);
	}

	std::pmr::string getPath(const std::string_view l_id) {
		std::pmr::string tempId(l_id, LocalMemoryManager::getManager().getAlloc());
		auto path{ m_paths.find(tempId) };
		return (path != m_paths.end() ? std::pmr::string(path->second, LocalMemoryManager::getManager().getAlloc()) : std::pmr::string("", LocalMemoryManager::getManager().getAlloc()));
	}

	bool requireResource(const std::string_view l_id) {
		auto res{ find(l_id) };
		if (res) [[likely]] {
			++res->second;
			return true;
		}

		auto path{ m_paths.find(l_id.data()) };
		if (path == m_paths.end()) [[likely]] {
			return false;
		}

		T* resource{ load(path->second) };
		if (!resource) [[likely]] {
			return false;
		}

		m_resources.emplace(l_id, std::make_pair(resource, 1u));
		return true;
	}

	bool releaseResource(const std::string_view l_id) {
		auto res{ find(l_id) };
		if (!res) [[likely]] {
			return false;
		}

		--res->second;
		if (!res->second) [[likely]] {
			unload(l_id);
		}

		return true;
	}

	T* load(const std::string_view l_path) {
		return static_cast<Derived*>(this)->load(l_path);
	}
};