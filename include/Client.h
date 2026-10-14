#pragma once

#include <vector>
#include "Player.h"
#include "Java/Pack.h"
#ifdef _WIN32
#include <winsock2.h>
#define Socket SOCKET
#else
#define Socket int
#endif

struct Client {
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

	int operator==(const Client& other) {
		return socket == other.socket;
	}

	int operator==(const Client&& other) {
		return socket == other.socket;
	}

	Type type;
	Socket socket;
	Status status;
	Player player;
	std::pair<uint64_t, uint64_t> sessionUUID = { -1, -1 };
	std::vector<Java::Pack> packs;
};
