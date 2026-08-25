#pragma once
#include <string>

namespace gtc::core {

struct WireGuardConfig {
    std::string privateKey;
    std::string address;
    std::string peerPublicKey;
    std::string endpoint;      // host:port
    std::string allowedIps = "0.0.0.0/0";

    bool isValid() const {
        return !privateKey.empty() && !peerPublicKey.empty() && !endpoint.empty();
    }
};

}
