// Modern way to do it(since 2017)
#pragma once
#include <filesystem>
#include <string>
#include "MemoryManager.h"

namespace utils {
	inline std::pmr::string getWorkingDirectory() {
		auto path{ std::filesystem::current_path().string() };
		path += static_cast<char>(std::filesystem::path::preferred_separator);
		return std::pmr::string(path, LocalMemoryManager::getManager().getAlloc());
	}
}

// Until 2017
/*#pragma once
#define RUNNING_WINDOWS
#include <string>
#include "MemoryManager.h"

namespace utils {
#ifdef RUNNING_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Shlwapi.h>
#include <string.h>
#pragma comment(lib, "Shlwapi.lib")

	inline std::pmr::string getWorkingDirectory() {
		HMODULE hModule{ GetModuleHandleA(nullptr) };
		if (hModule) [[likely]] {
			char path[256];
			GetModuleFileNameA(hModule, path, sizeof(path));
			PathRemoveFileSpecA(path);
			strcat_s(path, "\\");
			return std::pmr::string(path, LocalMemoryManager::getManager().getAlloc());
		}

		return "";
	}
#elif defined RUNNING_LINUX
#include <unistd.h>

	inline std::pmr::string getWorkingDirectory() {
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)) != nullptr) [[likely]] {
			return std::pmr::string(cwd, LocalMemoryManager::getManager().getAlloc()) + std::pmr::string("/", LocalMemoryManager::getManager().getAlloc());
		}

		return "";
	}

#endif
}
*/