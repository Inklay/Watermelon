#include "Client.h"

bool Client::hasPack(const std::string& name) {
	for (const auto& pack : packs) {
		if (pack.nameSpace == name) {
			return true;
		}
	}

	return false;
}
