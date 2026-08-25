#include "wg_smoketest.hpp"
#include <Geode/Geode.hpp>

extern "C" {
    #include "wireguard_ffi.h"
}

using namespace geode::prelude;

namespace gtc::network {

void runWireGuardSmokeTest() {
    log::info("[GTC] Running BoringTun FFI smoke test...");

    x25519_key privateKey = x25519_secret_key();
    x25519_key publicKey = x25519_public_key(privateKey);

    const char* privB64 = x25519_key_to_base64(privateKey);
    const char* pubB64 = x25519_key_to_base64(publicKey);

    log::info("[GTC] Generated private key: {}", privB64);
    log::info("[GTC] Derived public key:    {}", pubB64);

    bool valid = check_base64_encoded_x25519_key(pubB64) != 0;
    log::info("[GTC] Public key self-validation: {}", valid ? "OK" : "FAILED");

    x25519_key_to_str_free(privB64);
    x25519_key_to_str_free(pubB64);

    log::info("[GTC] BoringTun FFI smoke test complete.");
}

}
