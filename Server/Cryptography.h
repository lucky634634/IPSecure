#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <sha.h>
#include <hex.h>
#include <cryptlib.h>
#include <files.h>
#include <aes.h>
#include <osrng.h>
#include <dh.h>
#include <filter.h>
#include <modes.h>
#include <integer.h>
#include <eccrypto.h>
#include <asn.h>
#include <base64.h>

#pragma comment(lib, "cryptlib.lib")

std::string GetSHA1(const std::string& input);
void CreateAESKey(std::string& key, std::string& iv);
std::string EncryptAES(const std::string& input, const std::string& key, const std::string& iv);
std::string DecryptAES(const std::string& input, const std::string& key, const std::string& iv);