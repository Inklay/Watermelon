#include <Crypto/CFB8.h>
#include <stdexcept>

namespace Crypto {
	namespace CFB8 {
        Cipher::Cipher()
            : m_encryptCtx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free),
            m_decryptCtx(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free)
        {
            if (!m_encryptCtx || !m_decryptCtx) {
                throw std::runtime_error( "AES context allocation failed");
            }
        }

        void Cipher::init(const std::vector<uint8_t>& secret) {
            if (secret.size() != 16) {
                throw std::invalid_argument("Minecraft shared secret must be 16 bytes");
            }

            if (EVP_EncryptInit_ex(m_encryptCtx.get(), EVP_aes_128_cfb8(), nullptr,  secret.data(), secret.data()) != 1) {
                throw std::runtime_error("AES encryption initialization failed");
            }

            if (EVP_DecryptInit_ex(m_decryptCtx.get(), EVP_aes_128_cfb8(), nullptr, secret.data(), secret.data()) != 1) {
                throw std::runtime_error("AES decryption initialization failed");
            }
        }

        std::vector<uint8_t> Cipher::encrypt(const std::vector<uint8_t>& data) {
            if (data.empty()) {
                return {};
            }

            if (data.size() > static_cast<size_t>(INT_MAX)) {
                throw std::length_error("AES input too large");
            }

            std::vector<uint8_t> output(data.size() + 16);
            int outputLength = 0;

            if (EVP_EncryptUpdate(m_encryptCtx.get(), output.data(), &outputLength, data.data(), static_cast<int>(data.size())) != 1) {
                throw std::runtime_error("AES encryption failed");
            }

            output.resize(static_cast<size_t>(outputLength));
            return output;
        }

        std::vector<uint8_t> Cipher::decrypt(
            const std::vector<uint8_t>& data) {
            if (data.empty()) {
                return {};
            }

            if (data.size() > static_cast<size_t>(INT_MAX)) {
                throw std::length_error("AES input too large");
            }

            std::vector<uint8_t> output(data.size() + 16);
            int outputLength = 0;

            if (EVP_DecryptUpdate(m_decryptCtx.get(), output.data(), &outputLength, data.data(), static_cast<int>(data.size())) != 1) {
                throw std::runtime_error("AES decryption failed");
            }

            output.resize(static_cast<size_t>(outputLength));
            return output;
        }
	}
}
