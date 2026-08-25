#pragma once
#include <Geode/Geode.hpp>
#include "wg_config.hpp"
#include "vless_config.hpp"

using namespace geode::prelude;

namespace gtc::core {

inline void saveWireGuardConfig(const WireGuardConfig& cfg) {
    Mod::get()->setSavedValue<std::string>("wg-private-key", cfg.privateKey);
    Mod::get()->setSavedValue<std::string>("wg-address", cfg.address);
    Mod::get()->setSavedValue<std::string>("wg-peer-public-key", cfg.peerPublicKey);
    Mod::get()->setSavedValue<std::string>("wg-endpoint", cfg.endpoint);
    Mod::get()->setSavedValue<std::string>("wg-allowed-ips", cfg.allowedIps);
}

inline WireGuardConfig loadWireGuardConfig() {
    WireGuardConfig cfg;
    cfg.privateKey = Mod::get()->getSavedValue<std::string>("wg-private-key", "");
    cfg.address = Mod::get()->getSavedValue<std::string>("wg-address", "");
    cfg.peerPublicKey = Mod::get()->getSavedValue<std::string>("wg-peer-public-key", "");
    cfg.endpoint = Mod::get()->getSavedValue<std::string>("wg-endpoint", "");
    cfg.allowedIps = Mod::get()->getSavedValue<std::string>("wg-allowed-ips", "0.0.0.0/0");
    return cfg;
}

inline void saveVlessConfig(const VlessConfig& cfg) {
    Mod::get()->setSavedValue<std::string>("vless-uri", fmt::format(
        "vless://{}@{}:{}?security={}&sni={}&pbk={}&sid={}#{}",
        cfg.uuid, cfg.host, cfg.port, cfg.security, cfg.sni, cfg.publicKey, cfg.shortId, cfg.remark
    ));
}

inline std::optional<VlessConfig> loadVlessConfig() {
    auto raw = Mod::get()->getSavedValue<std::string>("vless-uri", "");
    if (raw.empty()) return std::nullopt;
    return VlessConfig::parse(raw);
}

}
