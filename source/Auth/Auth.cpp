#include "Auth/Auth.h"
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <httplib.h>
#include "Crypto/Crypto.h"
#include "Core/Watermelon.h"
#include "Utils/JSON.h"

namespace Auth {
	Result authenticatePlayer(std::shared_ptr<Client> client, Watermelon* server) {
		const std::string digest = Crypto::minecraftDigest("", client->getSecret(), server->getKeys().getPublicKey());
		httplib::Client http("https://sessionserver.mojang.com");
		http.set_connection_timeout(5);
		http.set_read_timeout(5);
		httplib::Result res = http.Get("/session/minecraft/hasJoined?username=" + client->getTempUsername() + "&serverId=" + digest);

		if (!res) {
			return { SERVER_DOWN, "", "", {} };
		}

        if (res->status == 200) {
            try {
                const auto data = nlohmann::json::parse(res->body);

                if (!data.contains("id") || !data["id"].is_string() || !data.contains("name") || !data["name"].is_string()) {
					return { FAILED, "", "" };
                }

                std::string uuid = data["id"].get<std::string>();
				std::string username = data["name"].get<std::string>();
				std::vector<ProfileProperty> playerProperties;
				playerProperties.reserve(data["properties"].size());

				for (const auto& property : data["properties"]) {
					playerProperties.emplace_back(
						property["name"].get<std::string>(),
						property["value"].get<std::string>(),
						property.value("signature", std::string{})
					);
				}

				return { AUTHENTIFIED, username, uuid, playerProperties };
            } catch (const nlohmann::json::exception&) {
				return { FAILED, "", "", {} };
            }
		} else if (res->status == 204) {
			return { FAILED, "", "", {} };
		}

		return { SERVER_DOWN, "", "", {} };
	}
}
