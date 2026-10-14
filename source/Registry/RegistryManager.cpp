#include "Registry/Registrymanager.h"
#include "Registry/BuiltIn.h"
#include "Registry/Synchronised.h"

RegistryManager::RegistryManager() {
	m_registryList.reserve(Registries::BUILT_IN_REGISTRIES_COUNT);

	for (const Registry* reg : Registries::BUILT_IN_REGISTRIES) {
		m_registryList.push_back(*reg);
		m_registryMap[reg->name] = m_registryList.size() - 1;
	}

	for (const Registry* reg : Registries::SYNCHRONIZED_REGISTRIES) {
		m_registryList.push_back(*reg);
		m_registryMap[reg->name] = m_registryList.size() - 1;
	}
}

RegistryManager& RegistryManager::getInstance() {
	static RegistryManager instance;
	return instance;
}

Registry* RegistryManager::getRegistry(const std::string& name) {
	auto it = m_registryMap.find(name);
	if (it != m_registryMap.end()) {
		return &m_registryList[it->second];
	}
	return nullptr;
}

std::vector<const Registry*> RegistryManager::getSynchronizedRegistries() {
	std::vector<const Registry*> regs;

	for (const Registry& reg : m_registryList) {
		if (reg.type == Registry::SYNCHRONISED) {
			regs.emplace_back(&reg);
		}
	}

	return regs;
}
