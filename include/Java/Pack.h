#pragma once

#include <string>

namespace Java {
	struct Pack {
		std::string nameSpace;
		std::string id;
		std::string version;

		int operator==(const Pack& other) {
			return nameSpace == other.nameSpace && id == other.id && version == other.version;
		}

		int operator==(const Pack&& other) {
			return nameSpace == other.nameSpace && id == other.id && version == other.version;
		}
	};
};
