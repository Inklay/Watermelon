#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <memory>

class Entity : public std::enable_shared_from_this<Entity> {
public:
	struct Vec3 {
		double x;
		double y;
		double z;
	};

	struct Delta3 {
		uint16_t x;
		uint16_t y;
		uint16_t z;
	};

	struct ChunkPosition {
		int x;
		int z;

		bool operator==(const ChunkPosition& other) const {
			return x == other.x && z == other.z;
		}

		ChunkPosition(const Vec3& position);
		ChunkPosition(int x, int z);
	};

	struct Rotation {
		float yaw;
		float pitch;
		float headYaw;
	};

	enum Type {
		PLAYER = 159
	};

	Entity(uint32_t entityId, const Vec3& position, const Rotation& rotation, Type type, std::pair<uint64_t, uint64_t> uuid);
	virtual ~Entity();
	void despawn();
	uint32_t getEntityId() const;
	bool setPosition(Vec3 position);
	void setRotation(Rotation rotation);
	ChunkPosition getChunkPosition();
	std::pair<uint64_t, uint64_t> getUUID() const;
	Type getType() const;
	const Vec3& getPosition() const;
	const Vec3& getVelocity() const;
	const Rotation& getRotation() const;
	uint32_t getProtocolData() const;
	bool isOnGround() const;
	bool changedPosition() const;
	bool changedRotation() const;
	void tickEnd();
	Delta3 getPositionDelta() const;

protected:
	void updateChunk();

	std::pair<uint64_t, uint64_t> m_uuid;
	uint32_t m_entityId;
	Type m_type;
	Vec3 m_position;
	Vec3 m_velocity;
	Rotation m_rotation;
	ChunkPosition m_chunkPosition;
	uint32_t m_protocolData;
	bool m_onGround;
	bool m_changedPosition;
	bool m_changedRotation;
	Vec3 m_lastGoodPosition;
};

struct ChunkPositionHash {
	size_t operator()(const Entity::ChunkPosition& p) const noexcept {
		uint64_t key = (static_cast<uint64_t>(static_cast<uint32_t>(p.x)) << 32) | static_cast<uint32_t>(p.z);
		return std::hash<uint64_t>{}(key);
	}
};
