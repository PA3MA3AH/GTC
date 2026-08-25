#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <string>

namespace gtc::core {

// Blocking TCP connect-time measurement. Call from a background thread only.
inline int measureTcpPingMs(const std::string& host, uint16_t port, int timeoutMs = 2000) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
        return -1;
    }

    int pingMs = -1;
    for (auto p = result; p != nullptr; p = p->ai_next) {
        SOCKET s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s == INVALID_SOCKET) continue;

        u_long mode = 1;
        ioctlsocket(s, FIONBIO, &mode);

        auto start = std::chrono::steady_clock::now();
        connect(s, p->ai_addr, static_cast<int>(p->ai_addrlen));

        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(s, &writeSet);
        timeval tv{ timeoutMs / 1000, (timeoutMs % 1000) * 1000 };

        if (select(0, nullptr, &writeSet, nullptr, &tv) > 0) {
            int err = 0;
            int len = sizeof(err);
            getsockopt(s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &len);
            if (err == 0) {
                auto end = std::chrono::steady_clock::now();
                pingMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
            }
        }

        closesocket(s);
        if (pingMs >= 0) break;
    }

    freeaddrinfo(result);
    return pingMs;
}

}
