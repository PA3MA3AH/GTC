#pragma once
#include <string>
#include "../core/wg_config.hpp"

namespace gtc::network {

struct HandshakeTestResult {
    bool success = false;
    std::string message;
    int64_t timeSinceHandshakeMs = -1;
    float estimatedLoss = 0.f;
    int32_t estimatedRttMs = -1;
};

// Blocking. Call from a background thread only.
HandshakeTestResult testWireGuardHandshake(const gtc::core::WireGuardConfig& cfg);

}
