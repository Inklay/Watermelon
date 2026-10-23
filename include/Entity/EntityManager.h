#pragma once

#include "Entity/Entity.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>

class EntityManager {
public:
	EntityManager(const EntityManager&) = delete;
	EntityManager& operator=(const EntityManager&) = delete;
	void createEntity(const std::shared_ptr<Entity>& entity);
	void deleteEntity(const std::shared_ptr<Entity>& entity);
	const std::shared_ptr<Entity> getEntity(uint32_t entityId);
	void moveEntity(const Entity::ChunkPosition& oldChunkPosition, const Entity::ChunkPosition& newChunkPosition, uint32_t entityId);
	void getEntityAround(const Entity::ChunkPosition& chunkPosition, std::vector<uint32_t>& out, int distance);
	static EntityManager& getInstance();

private:
	EntityManager();
	void addToPosMap(const Entity::ChunkPosition& chunkPosition, uint32_t entityId);
	void removeFromPosMap(const Entity::ChunkPosition& chunkPosition, uint32_t entityId);

	std::mutex m_mutex;
	std::unordered_map<Entity::ChunkPosition, std::unordered_set<uint32_t>, ChunkPositionHash> m_posMap;
	std::unordered_map<uint32_t, std::shared_ptr<Entity>> m_map;
};
