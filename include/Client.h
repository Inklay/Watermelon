#pragma once

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
};
