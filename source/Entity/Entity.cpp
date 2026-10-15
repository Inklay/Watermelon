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
