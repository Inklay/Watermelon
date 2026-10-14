#pragma once

#include "Registry/Registry.h"
#include <unordered_map>

class RegistryManager {
public:
	RegistryManager(const RegistryManager&) = delete;
	RegistryManager(const RegistryManager&&) = delete;
	RegistryManager& operator=(const RegistryManager&) = delete;
	RegistryManager& operator=(const RegistryManager&&) = delete;
	static RegistryManager& getInstance();
	Registry* getRegistry(const std::string& name);
	std::vector<const Registry*> getSynchronizedRegistries();

private:
	RegistryManager();

	std::vector<Registry> m_registryList;
	std::unordered_map<std::string, size_t> m_registryMap;
};
