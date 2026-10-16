#include "Entity/Entity.h"

Entity::Entity(int32_t entityId, Position position, Rotation rotation, Type type) :
	m_entityId(entityId),
	m_type(type),
	m_position(position),
	m_rotation(rotation)
{
}

int32_t Entity::getEntityId() {
	return m_entityId;
}

void Entity::setPosition(Position position) {
	m_position = position;
}

void Entity::setRotation(Rotation rotation) {
	m_rotation = rotation;
}
