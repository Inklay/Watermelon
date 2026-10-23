#include "Entity/Entity.h"
#include <cmath>
#include "Entity/EntityManager.h"

Entity::Entity(uint32_t entityId, const Vec3& position, const Rotation& rotation, Type type, std::pair<uint64_t, uint64_t> uuid) :
	m_uuid(uuid),
	m_entityId(entityId),
	m_type(type),
	m_position(position),
	m_velocity({0, 0, 0}),
	m_rotation(rotation),
	m_chunkPosition(m_position),
	m_protocolData(0)
{
	EntityManager::getInstance().createEntity(this);
}

Entity::~Entity() {
	EntityManager::getInstance().deleteEntity(this);
}

uint32_t Entity::getEntityId() const {
	return m_entityId;
}

bool Entity::setPosition(Vec3 position) {
	m_position = position;
	updateChunk();
	return true;
}

void Entity::setRotation(Rotation rotation) {
	m_rotation = rotation;
}

void Entity::updateChunk() {
	ChunkPosition newPos(m_position);

	if (m_chunkPosition == newPos) {
		return;
	}

	EntityManager::getInstance().moveEntity(m_chunkPosition, newPos, m_entityId);
	m_chunkPosition = newPos;
}

Entity::ChunkPosition Entity::getChunkPosition() {
	return m_chunkPosition;
}

Entity::ChunkPosition::ChunkPosition(const Entity::Vec3& position) {
	x = static_cast<int>(std::floor(position.x / 16));
	z = static_cast<int>(std::floor(position.z / 16));
}

Entity::ChunkPosition::ChunkPosition(int x, int z) :
	x(x),
	z(z)
{}

std::pair<uint64_t, uint64_t> Entity::getUUID() const {
	return m_uuid;
}

Entity::Type Entity::getType() const {
	return m_type;
}

const Entity::Vec3& Entity::getPosition() const {
	return m_position;
}

const Entity::Vec3& Entity::getVelocity() const {
	return m_velocity;
}

const Entity::Rotation& Entity::getRotation() const {
	return m_rotation;
}

uint32_t Entity::getProtocolData() const {
	return m_protocolData;
}
