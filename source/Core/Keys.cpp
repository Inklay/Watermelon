#include "Core/Keys.h"
#include <nlohmann/json.hpp>
#include "Utils/Base64.h"
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <httplib.h>

Keys::Keys(Logger& logger, bool keyMojangPublicKeys) :
	m_key(Crypto::RSA::generateKeyPair())
{
	if (keyMojangPublicKeys) {
		fetchMojangPublicKeys(logger);
	}
}

void Keys::fetchMojangPublicKeys(Logger& logger) {
	httplib::Client http("https://api.minecraftservices.com");
	http.set_connection_timeout(5);
	http.set_read_timeout(5);
	httplib::Result res = http.Get("/publickeys");

	if (!res || res->status != 200) {
		logger.log("Can't get Mojang's public key. Chat messages can't be secured", "Watermelon", Logger::WARN);
	}

	try {
		const auto data = nlohmann::json::parse(res->body);
		const auto& keys = data.at("playerCertificateKeys");

		if (!keys.is_array() || keys.empty())
			return;

		for (const auto& entry : keys) {
			const std::string encoded = entry.at("publicKey").get<std::string>();
			std::vector<uint8_t> decoded = Utils::Base64::decode(encoded);
			m_mojangPublicKeys.emplace_back(Crypto::DER2EVP(decoded));
		}
	} catch (...) {
		logger.log("Can't get Mojang's public key. Chat messages can't be secured", "Watermelon", Logger::WARN);
	}
}

std::vector<uint8_t> Keys::getPublicKey() {
	return Crypto::RSA::publicKeyDER(m_key);
}

std::vector<uint8_t> Keys::decryptData(const std::vector<uint8_t>& data) {
	return Crypto::RSA::decrypt(data, m_key);
}

const std::vector<Crypto::Key>& Keys::getMojangPublicKeys() {
	return m_mojangPublicKeys;
}

bool Keys::checkChatSession(const std::vector<uint8_t>& signedData, const std::vector<uint8_t>& signature) {
	for (const auto& key : m_mojangPublicKeys) {
		EVP_MD_CTX* ctx = EVP_MD_CTX_new();

		if (!ctx) {
			return false;
		}

		bool valid = false;

		if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha1(), nullptr, key.get()) == 1) {
			valid = EVP_DigestVerify(ctx, signature.data(), signature.size(), signedData.data(), signedData.size()) == 1;
		}

		EVP_MD_CTX_free(ctx);

		if (valid) {
			return true;
		}
	}

	return false;
}
