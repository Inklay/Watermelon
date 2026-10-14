#pragma once

#include <string>
#include <vector>

struct RegistryTag {
	struct Tag {
		std::string name;
		std::vector<int32_t> values;
	};

	std::string registryName;
	std::vector<Tag> tags;
};
