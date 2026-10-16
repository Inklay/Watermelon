#pragma once

#include <vector>
#include <mutex>
#include "Player/Player.h"
#include "Java/Pack.h"
#ifdef _WIN32
#include <winsock2.h>
#define Socket SOCKET
#else
#define Socket int
#endif

class Watermelon;

class Client {
public:
	enum Type {
		JAVA,
		BEDROCK
	};

	enum Status {
		HANDSHAKING,
		STATUS,
		LOGIN,
		CONFIGURATION,
		PLAY
	};

	enum PacketStatus {
		INCOMPLETE,
		COMPLETE,
		INVALID
	};

	int operator==(const Client& other) {
		return m_socket == other.m_socket;
	}

	int operator==(const Client&& other) {
		return m_socket == other.m_socket;
	}

	Client(Type type, Socket socket, Watermelon* server);
	~Client();
	Socket getSocket() const;
	Type getType() const;
	bool hasPack(const std::string& name);
	void changeStatus(Status status);
	Status getStatus();
	bool createPlayer(const std::string& username, std::pair<uint64_t, uint64_t> uuid, int32_t entityId, Entity::Position position, Entity::Rotation rotation);
	Player* getPlayer();
	bool hasPlayer();
	void setPacks(const std::vector<Java::Pack> packs);
	std::vector<Java::Pack> getPacks();
	void setSessionUUID(std::pair<uint64_t, uint64_t> sessionUUID);
	PacketStatus readPacket(std::vector<uint8_t>& packet);
	void appendReceivedData(const uint8_t* data, size_t size);
	void disconnect();
	void sendData(const std::vector<char>& data);

private:
	const Type m_type;
	Status m_status;
	Socket m_socket;
	Player* m_player;
	std::pair<uint64_t, uint64_t> m_sessionUUID = { -1, -1 };
	std::vector<Java::Pack> m_packs;
	std::mutex m_mutex;
	std::mutex m_sendingMutex;
	std::vector<uint8_t> m_receiveBuffer;
	Watermelon* m_server;
	bool m_disconnected;
};
