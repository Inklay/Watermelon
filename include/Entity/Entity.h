#pragma once

#include <cstdint>

class Entity {
public:
	struct Position {
		double x;
		double y;
		double z;
	};

	struct Rotation {
		float yaw;
		float pitch;
	};

	enum Type {
		PLAYER
	};

	Entity(int32_t entityId, Position position, Rotation rotation, Type type);
	int32_t getEntityId();
	void setPosition(Position position);
	void setRotation(Rotation rotation);

protected:
	int32_t m_entityId;
	Type m_type;
	Position m_position;
	Rotation m_rotation;
};
