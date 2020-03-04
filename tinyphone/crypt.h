#pragma once

#ifndef CRYPT_HEADER_FILE_H
#define CRYPT_HEADER_FILE_H

#include <iostream>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>

class EncryptDecrypt {
	private :
	std::string key, iv;

	std::string encrypt(const std::string& str_in, const std::string& key, const std::string& iv){
		std::string str_out;
		CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryption((CryptoPP::byte*)key.c_str(),    key.length(), (CryptoPP::byte*)iv.c_str());
		CryptoPP::StringSource encryptor(str_in, true,
			new CryptoPP::StreamTransformationFilter(encryption,
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(str_out),
					false // do not append a newline
				)
			)
		);
		return str_out;
	}

	std::string decrypt(const std::string& str_in, const std::string& key, const std::string& iv){

		std::string str_out;    
		CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryption((CryptoPP::byte*)key.c_str(), key.length(), (CryptoPP::byte*)iv.c_str());

		CryptoPP::StringSource decryptor(str_in, true,
			new CryptoPP::Base64Decoder(
				new CryptoPP::StreamTransformationFilter(decryption,
					new CryptoPP::StringSink(str_out)
				)
			)
		);
		return str_out;
	}

	public:
	EncryptDecrypt(std::string sKey, std::string _iv){
		if(CryptoPP::AES::DEFAULT_KEYLENGTH < sKey.size())
            sKey = sKey.substr(0, CryptoPP::AES::DEFAULT_KEYLENGTH); // chop if too long
        else if(CryptoPP::AES::DEFAULT_KEYLENGTH > sKey.size())
            sKey += std::string(CryptoPP::AES::DEFAULT_KEYLENGTH - sKey.size(), '*'); // pad
		key = sKey;
		iv = _iv;
	}
  
    std::string Encrypt(std::string input){
		return encrypt(input, key, iv);
    }

    std::string Decrypt(std::string base64){
		return decrypt(base64, key, iv);
    }

};


#endif