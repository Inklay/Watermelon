#include "Entity/EntityManager.h"

EntityManager::EntityManager()
{}

void EntityManager::addToPosMap(const Entity::ChunkPosition& chunkPosition, uint32_t entityId) {
	m_posMap[chunkPosition].insert(entityId);
}

void EntityManager::removeFromPosMap(const Entity::ChunkPosition& chunkPosition, uint32_t entityId) {
	auto it = m_posMap.find(chunkPosition);

	if (it == m_posMap.end()) {
		return;
	}

	it->second.erase(entityId);
	if (it->second.empty()) {
		m_posMap.erase(it);
	}
}

void EntityManager::moveEntity(const Entity::ChunkPosition& oldChunkPosition, const Entity::ChunkPosition& newChunkPosition, uint32_t entityId) {
	if (oldChunkPosition == newChunkPosition) {
		return;
	}

	std::lock_guard<std::mutex> lock(m_mutex);
	removeFromPosMap(oldChunkPosition, entityId);
	addToPosMap(newChunkPosition, entityId);
}

void EntityManager::getEntityAround(const Entity::ChunkPosition& chunkPosition, std::vector<uint32_t>& out, int distance) {
	std::lock_guard<std::mutex> lock(m_mutex);
	for (int x = chunkPosition.x - distance; x <= chunkPosition.x + distance; x++) {
		for (int z = chunkPosition.z - distance; z <= chunkPosition.z + distance; z++) {
			auto it = m_posMap.find({x, z});

			if (it == m_posMap.end()) {
				continue;
			}

			out.insert(out.end(), it->second.begin(), it->second.end());
		}
	}
}

EntityManager& EntityManager::getInstance() {
	static EntityManager instance;
	return instance;
}

void EntityManager::createEntity(Entity* entity) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_map[entity->getEntityId()] = entity;
	addToPosMap(entity->getChunkPosition(), entity->getEntityId());
}

void EntityManager::deleteEntity(Entity* entity) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_map.erase(entity->getEntityId());
	removeFromPosMap(entity->getChunkPosition(), entity->getEntityId());
}

Entity* EntityManager::getEntity(uint32_t entityId) {
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_map.find(entityId);

	if (it == m_map.end()) {
		return nullptr;
	}

	return it->second;
}
