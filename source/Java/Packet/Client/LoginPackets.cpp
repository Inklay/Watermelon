#include "Utils/UUID.h"
#include "Java/Packet/Packet.h"
#include "Java/Packet/Client/StatusPackets.h"

namespace Java {
	namespace Packet {
		namespace Client {
			namespace Login {
				void disconnect(const std::shared_ptr<::Client>& client, const std::string& reason) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::DISCONNECT, buffer);
					Java::Packet::writeString(reason, buffer);
					client->sendData(buffer);
				}

				void encryptionRequest(const std::shared_ptr<::Client>& client, Watermelon* server) {
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

					client->sendData(buffer);
				}

				void loginSuccess(const std::shared_ptr<::Client>& client) {
					std::vector<uint8_t> buffer;
					std::pair<uint64_t, uint64_t> sessionUUID = Utils::UUID::generateSessionUUID();
					Player::Player* player = client->getPlayer();

					client->setSessionUUID(sessionUUID);

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::LOGIN_SUCCESS, buffer);
					Java::Packet::writeUUID(player->getUUID(), buffer);
					Java::Packet::writeString(player->getUsername(), buffer);
					Java::Packet::writeProfileProperties(player->getProfileProperties(), buffer);
					Java::Packet::writeUUID(sessionUUID, buffer);

					client->sendData(buffer);
				}

				void setCompression(const std::shared_ptr<::Client>& client) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::SET_COMPRESSION, buffer);
					Java::Packet::writeVarInt(256, buffer);

					client->sendData(buffer);
					client->enableCompression();
				}

				void loginPluginRequest(const std::shared_ptr<::Client>& client, const std::string& identifier) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::LOGIN_PLUGIN_REQUEST, buffer);
					Java::Packet::writeVarInt(std::rand(), buffer);
					Java::Packet::writeString(identifier, buffer);

					client->sendData(buffer);
				}

				void cookieRequest(const std::shared_ptr<::Client>& client, const std::string& key) {
					std::vector<uint8_t> buffer;

					Java::Packet::writeVarInt((int32_t)Java::Packet::PacketId::LoginClient::COOKIE_REQUEST, buffer);
					Java::Packet::writeString(key, buffer);

					client->sendData(buffer);
				}
			};
		};
	};
};
