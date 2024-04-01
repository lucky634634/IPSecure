#include "Cryptography.h"

using namespace CryptoPP;

std::string GetSHA1(const std::string& input)
{
	std::string output;
	SHA1 hash;
	hash.Update((const byte*)input.data(), input.size());
	output.resize(hash.DigestSize());
	hash.Final((byte*)&output[0]);
	std::stringstream ss;
	HexEncoder encoder(new FileSink(ss));
	StringSource strsrc(output, true, new Redirector(encoder));
	ss >> output;

	return output;
}

void CreateAESKey(std::string& key, std::string& iv)
{
	AutoSeededRandomPool prng;
	byte keyBytes[AES::DEFAULT_KEYLENGTH];
	prng.GenerateBlock(keyBytes, sizeof(keyBytes));

	// Generate a random IV
	byte ivBytes[AES::BLOCKSIZE];
	prng.GenerateBlock(ivBytes, sizeof(ivBytes));

	StringSource(keyBytes, sizeof(keyBytes), true, new Base64Encoder(new StringSink(key)));
	StringSource(ivBytes, sizeof(ivBytes), true, new Base64Encoder(new StringSink(iv)));
}

std::string EncryptAES(const std::string& input, const std::string& key, const std::string& iv)
{
	byte keyBytes[AES::DEFAULT_KEYLENGTH];
	byte ivBytes[AES::BLOCKSIZE];
	StringSource(key, true, new Base64Decoder(new ArraySink(keyBytes, sizeof(keyBytes))));
	StringSource(iv, true, new Base64Decoder(new ArraySink(ivBytes, sizeof(ivBytes))));

	CBC_Mode< AES >::Encryption encryptor;
	encryptor.SetKeyWithIV(keyBytes, sizeof(keyBytes), ivBytes);

	std::string ciphertext;
	StringSource(input, true,
		new StreamTransformationFilter(encryptor,
			new StringSink(ciphertext)
		)
	);

	// Encode to Base64 for easy transport
	std::string encoded;
	StringSource(ciphertext, true,
		new Base64Encoder(
			new StringSink(encoded)
		)
	);
	return encoded;
}

std::string DecryptAES(const std::string& input, const std::string& key, const std::string& iv)
{
	byte keyBytes[AES::DEFAULT_KEYLENGTH];
	byte ivBytes[AES::BLOCKSIZE];
	StringSource(key, true, new Base64Decoder(new ArraySink(keyBytes, sizeof(keyBytes))));
	StringSource(iv, true, new Base64Decoder(new ArraySink(ivBytes, sizeof(ivBytes))));

	CBC_Mode< AES >::Decryption decryptor;
	decryptor.SetKeyWithIV(keyBytes, sizeof(keyBytes), ivBytes);

	// Decrypt
	std::string decrypted;
	StringSource(input, true,
		new Base64Decoder(
			new StreamTransformationFilter(decryptor,
				new StringSink(decrypted)
			)
		)
	);
	return decrypted;
}
