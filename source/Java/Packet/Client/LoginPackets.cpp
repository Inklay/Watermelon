#include "Utils/UUID.h"
#include "Java/Packet/Packet.h"
#include "Java/Packet/Client/StatusPackets.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Login {
				void disconnect(std::shared_ptr<::Client> client, Watermelon* server, const std::string& reason) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::DISCONNECT, buffer);
					Java::Packet::writeString(reason, buffer);
					server->sendDataToClient(client, buffer);
					server->disconnectClient(client.get());
				}

				void encryptionRequest(std::shared_ptr<::Client> client, Watermelon* server) {
					std::vector<uint8_t> buffer;
					std::vector<uint8_t> key = server->getKeys().getPublicKey();
					std::vector<uint8_t> token = client->generateEncryptionToken();

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::ENCRYPTION_REQUEST, buffer);
					Java::Packet::writeString("", buffer); // server ID is always empty on vanilla server
					Java::Packet::writeVarInt((int32_t)key.size(), buffer);
					buffer.insert(buffer.end(), key.begin(), key.end());
					Java::Packet::writeVarInt(8, buffer);
					buffer.insert(buffer.end(), token.begin(), token.end());
					Java::Packet::writeBool(server->isOnlineMode(), buffer);

					server->sendDataToClient(client, buffer);
				}

				void loginSuccess(std::shared_ptr<::Client> client, Watermelon* server, const std::string& username, std::pair<uint64_t, uint64_t> uuid) {
					std::vector<uint8_t> buffer;
					std::pair<uint64_t, uint64_t> sessionUUID = Utils::UUID::generateSessionUUID();

					client->setSessionUUID(sessionUUID);

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::LOGIN_SUCCESS, buffer);
					Java::Packet::writeUUID(uuid, buffer);
					Java::Packet::writeString(username, buffer);
					Java::Packet::writeVarInt(0, buffer); // FIXME game profile properties
					Java::Packet::writeUUID(sessionUUID, buffer);

					server->sendDataToClient(client, buffer);
				}
			};
		};
	};
};
