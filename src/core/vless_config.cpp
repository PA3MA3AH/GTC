#include "vless_config.hpp"
#include <sstream>

namespace gtc::core {

static std::string urlDecode(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) {
            int value = std::stoi(s.substr(i + 1, 2), nullptr, 16);
            out += static_cast<char>(value);
            i += 2;
        } else if (s[i] == '+') {
            out += ' ';
        } else {
            out += s[i];
        }
    }
    return out;
}

std::optional<VlessConfig> VlessConfig::parse(const std::string& uri) {
    const std::string prefix = "vless://";
    if (uri.rfind(prefix, 0) != 0) return std::nullopt;

    std::string rest = uri.substr(prefix.size());

    std::string remark;
    if (auto hashPos = rest.find('#'); hashPos != std::string::npos) {
        remark = urlDecode(rest.substr(hashPos + 1));
        rest = rest.substr(0, hashPos);
    }

    std::string query;
    if (auto qPos = rest.find('?'); qPos != std::string::npos) {
        query = rest.substr(qPos + 1);
        rest = rest.substr(0, qPos);
    }

    auto atPos = rest.find('@');
    if (atPos == std::string::npos) return std::nullopt;

    std::string uuid = rest.substr(0, atPos);
    std::string hostPort = rest.substr(atPos + 1);

    auto colonPos = hostPort.rfind(':');
    if (colonPos == std::string::npos) return std::nullopt;

    VlessConfig cfg;
    cfg.uuid = uuid;
    cfg.host = hostPort.substr(0, colonPos);
    cfg.port = static_cast<uint16_t>(std::stoi(hostPort.substr(colonPos + 1)));
    cfg.remark = remark;

    std::istringstream qs(query);
    std::string pair;
    while (std::getline(qs, pair, '&')) {
        auto eq = pair.find('=');
        if (eq == std::string::npos) continue;
        std::string key = pair.substr(0, eq);
        std::string value = urlDecode(pair.substr(eq + 1));

        if (key == "security") cfg.security = value;
        else if (key == "sni") cfg.sni = value;
        else if (key == "pbk") cfg.publicKey = value;
        else if (key == "sid") cfg.shortId = value;
    }

    return cfg;
}

}
