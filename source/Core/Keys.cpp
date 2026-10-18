#include "Core/Keys.h"

Keys::Keys() :
	m_key(Crypto::RSA::generateKeyPair())
{
}

std::vector<uint8_t> Keys::getPublicKey() {
	return Crypto::RSA::publicKeyDER(m_key);
}

std::vector<uint8_t> Keys::decryptData(const std::vector<uint8_t>& data) {
	return Crypto::RSA::decrypt(data, m_key);
}
