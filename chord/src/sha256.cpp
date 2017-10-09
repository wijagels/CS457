#include "sha256.hpp"
#include <cstdio>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <sstream>
#include <string>

std::string sha256(const std::string &message) {
  unsigned char digest[SHA256_DIGEST_LENGTH];
  auto md = EVP_sha256();
  auto mdctx = EVP_MD_CTX_create();
  unsigned int digest_len;
  EVP_DigestInit_ex(mdctx, md, nullptr);
  EVP_DigestUpdate(mdctx, message.c_str(), message.size());
  EVP_DigestFinal_ex(mdctx, digest, &digest_len);
  EVP_MD_CTX_destroy(mdctx);
  std::ostringstream os;
  for (auto i = 0u; i < digest_len; i++) {
    char buf[3];
    std::snprintf(buf, sizeof(buf), "%02x", digest[i]);
    os << buf;
  }
  return os.str();
}
