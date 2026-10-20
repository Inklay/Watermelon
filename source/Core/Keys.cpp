#include "Core/Keys.h"
#include "Utils/JSON.h"
#include "Utils/Base64.h"
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <httplib.h>

Keys::Keys() :
	m_key(Crypto::RSA::generateKeyPair())
{
	httplib::Client http("https://api.minecraftservices.com");
	http.set_connection_timeout(5);
	http.set_read_timeout(5);
	httplib::Result res = http.Get("/publickeys");

	if (!res || res->status != 200) {
		return;
	}

	const auto data = nlohmann::json::parse(res->body);
	const auto& keys = data.at("playerCertificateKeys");

	if (!keys.is_array() || keys.empty())
		return;

	for (const auto& entry : keys) {
		const std::string encoded = entry.at("publicKey").get<std::string>();
		std::vector<uint8_t> decoded = Utils::Base64::decode(encoded);
		m_mojangPublicKeys.push_back(std::move(Crypto::DER2EVP(decoded)));
	}
}

std::vector<uint8_t> Keys::getPublicKey() {
	return Crypto::RSA::publicKeyDER(m_key);
}

std::vector<uint8_t> Keys::decryptData(const std::vector<uint8_t>& data) {
	return Crypto::RSA::decrypt(data, m_key);
}

std::vector<Crypto::Key> Keys::getMojangPublicKeys() {
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
