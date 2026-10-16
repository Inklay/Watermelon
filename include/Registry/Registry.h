#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct Registry {
	enum Type {
		BUILT_IN,
		SYNCHRONISED
	};

	struct Entry {
		int32_t protocolId;
		std::string textId;
		std::vector<uint8_t> nbtPayload;
	};

	std::string name;
	Type type;
	std::vector<Entry> entries;
};
