#include "../headers/SHA.hpp"
//#include <cryptopp>
#include <cryptopp/cryptlib.h>
#include <iostream>

#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

using namespace std;

std::string SHA_512_digest(std::string message) {
    CryptoPP::SHA512 hash;
    byte digest[ CryptoPP::SHA512::DIGESTSIZE ];
    //std::string message = "abcdefghijklmnopqrstuvwxyz";

    hash.CalculateDigest( digest, (byte*) message.c_str(), message.length() );

    CryptoPP::HexEncoder encoder;
    std::string output;
    encoder.Attach( new CryptoPP::StringSink( output ) );
    encoder.Put( digest, sizeof(digest) );
    encoder.MessageEnd();

    //std::cout << output << std::endl;

    return output;
}

std::string SHA_256_digest(std::string message) {
    CryptoPP::SHA256 hash;
    byte digest[ CryptoPP::SHA256::DIGESTSIZE ];
    //std::string message = "abcdefghijklmnopqrstuvwxyz";

    hash.CalculateDigest( digest, (byte*) message.c_str(), message.length() );

    CryptoPP::HexEncoder encoder;
    std::string output;
    encoder.Attach( new CryptoPP::StringSink( output ) );
    encoder.Put( digest, sizeof(digest) );
    encoder.MessageEnd();

    //std::cout << output << std::endl;

    return output;
}