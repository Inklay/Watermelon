#pragma once

#include <string>
#include <cstdint>

namespace Utils {
	namespace UUID {
		std::string formatUUID(const std::pair<uint64_t, uint64_t>& uuid);
		std::pair<uint64_t, uint64_t> generateSessionUUID();
		std::pair<uint64_t, uint64_t> offlineUUID(const std::string& username);
		std::pair<uint64_t, uint64_t> fromString(const std::string& str);
	};
};
