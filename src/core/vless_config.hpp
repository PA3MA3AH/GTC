#pragma once
#include <string>
#include <optional>
#include <cstdint>

namespace gtc::core {

struct VlessConfig {
    std::string uuid;
    std::string host;
    uint16_t port = 443;
    std::string security; // "reality", "tls", "none"
    std::string sni;
    std::string publicKey; // pbk
    std::string shortId;   // sid
    std::string remark;

    bool isValid() const {
        return !uuid.empty() && !host.empty() && port != 0;
    }

    static std::optional<VlessConfig> parse(const std::string& uri);
};

}
