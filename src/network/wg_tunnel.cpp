#include "wg_tunnel.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <chrono>
#include <thread>

extern "C" {
    #include "wireguard_ffi.h"
}

using namespace gtc::core;

namespace gtc::network {

static bool parseEndpoint(const std::string& endpoint, std::string& host, std::string& port) {
    auto colonPos = endpoint.rfind(':');
    if (colonPos == std::string::npos) return false;
    host = endpoint.substr(0, colonPos);
    port = endpoint.substr(colonPos + 1);
    return !host.empty() && !port.empty();
}

HandshakeTestResult testWireGuardHandshake(const WireGuardConfig& cfg) {
    HandshakeTestResult result;

    std::string host, port;
    if (!parseEndpoint(cfg.endpoint, host, port)) {
        result.message = "Invalid endpoint format (expected host:port)";
        return result;
    }

    // --- 1. Create the tunnel object via BoringTun FFI ---
    wireguard_tunnel* tunnel = new_tunnel(
        cfg.privateKey.c_str(),
        cfg.peerPublicKey.c_str(),
        nullptr,      // preshared key, none for now
        25,           // keepalive seconds
        0             // session index prefix
    );

    if (!tunnel) {
        result.message = "new_tunnel() returned null - check that keys are valid base64";
        return result;
    }

    // --- 2. Resolve + connect UDP socket to the endpoint ---
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    addrinfo* addrResult = nullptr;
    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &addrResult) != 0) {
        tunnel_free(tunnel);
        result.message = "Failed to resolve endpoint host";
        return result;
    }

    SOCKET sock = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (sock == INVALID_SOCKET) {
        freeaddrinfo(addrResult);
        tunnel_free(tunnel);
        result.message = "Failed to create UDP socket";
        return result;
    }

    if (connect(sock, addrResult->ai_addr, static_cast<int>(addrResult->ai_addrlen)) != 0) {
        closesocket(sock);
        freeaddrinfo(addrResult);
        tunnel_free(tunnel);
        result.message = "Failed to connect UDP socket to endpoint";
        return result;
    }
    freeaddrinfo(addrResult);

    // Non-blocking with select-based timeout for recv
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    // --- 3. Send the handshake initiation packet ---
    std::vector<uint8_t> outBuf(2048);
    auto initResult = wireguard_force_handshake(tunnel, outBuf.data(), static_cast<uint32_t>(outBuf.size()));

    if (initResult.op != WRITE_TO_NETWORK || initResult.size == 0) {
        closesocket(sock);
        tunnel_free(tunnel);
        result.message = "Failed to produce handshake initiation packet";
        return result;
    }

    send(sock, reinterpret_cast<const char*>(outBuf.data()), static_cast<int>(initResult.size), 0);

    // --- 4. Wait for response, feed through wireguard_read, retry a few times ---
    std::vector<uint8_t> recvBuf(2048);
    std::vector<uint8_t> readOutBuf(2048);

    bool handshakeDone = false;
    for (int attempt = 0; attempt < 5 && !handshakeDone; attempt++) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(sock, &readSet);
        timeval tv{ 1, 0 }; // 1 second per attempt

        int selectResult = select(0, &readSet, nullptr, nullptr, &tv);
        if (selectResult <= 0) continue; // timeout, try again

        int received = recv(sock, reinterpret_cast<char*>(recvBuf.data()), static_cast<int>(recvBuf.size()), 0);
        if (received <= 0) continue;

        auto readResult = wireguard_read(
            tunnel, recvBuf.data(), static_cast<uint32_t>(received),
            readOutBuf.data(), static_cast<uint32_t>(readOutBuf.size())
        );

        if (readResult.op == WRITE_TO_NETWORK && readResult.size > 0) {
            send(sock, reinterpret_cast<const char*>(readOutBuf.data()), static_cast<int>(readResult.size), 0);
        } else if (readResult.op == WIREGUARD_ERROR) {
            result.message = "wireguard_read reported a protocol error";
            break;
        }

        // Check whether the handshake actually completed
        auto currentStats = wireguard_stats(tunnel);
        if (currentStats.time_since_last_handshake >= 0) {
            handshakeDone = true;
            result.success = true;
            result.timeSinceHandshakeMs = currentStats.time_since_last_handshake;
            result.estimatedLoss = currentStats.estimated_loss;
            result.estimatedRttMs = currentStats.estimated_rtt;
            result.message = "Handshake completed";
        }
    }

    if (!handshakeDone && result.message.empty()) {
        result.message = "No valid handshake response received (timeout after 5 attempts)";
    }

    closesocket(sock);
    tunnel_free(tunnel);
    return result;
}

}
