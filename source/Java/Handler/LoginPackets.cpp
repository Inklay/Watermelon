#include "Java/Handler/StatusPackets.h"
#include "Utils/JSON.h"
#include "Java/Packet.h"
#include "Utils/UUID.h"

using json = nlohmann::json;

namespace Java {
	namespace Handler {
		namespace Login {
			std::vector<char> loginStartPacket(std::shared_ptr<Client> client, Watermelon* server, const std::string& username, std::pair<uint64_t, uint64_t> uuid) {
				std::vector<char> buffer;

				std::pair<uint64_t, uint64_t> sessionUUID = Utils::UUID::generateSessionUUID();

				{
					std::lock_guard<std::mutex> lock(server->getClientMutex());
					client->player.username = username;
					client->player.uuid = uuid;
					client->sessionUUID = sessionUUID;
				}

				server->getLogger().log("UUID of player " + username + " is " + Utils::UUID::formatUUID(uuid), "Watermelon", Logger::INFO);

				if (server->getConfig()["networking"]["java"]["online_mode"].value_or(true)) {
					// encryption packet
				} else {
					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::LOGIN_SUCCESS, buffer);
					Java::Packet::writeUUID(uuid, buffer);
					Java::Packet::writeString(username, buffer);
					Java::Packet::writeVarInt(0, buffer);
					Java::Packet::writeUUID(sessionUUID, buffer);
				}

				return buffer;
			}
		};
	};
};
