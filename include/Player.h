#pragma once

#include <string>

struct Player {
	std::string username = "";
	std::pair<uint64_t, uint64_t> uuid = { -1, -1 };
};
