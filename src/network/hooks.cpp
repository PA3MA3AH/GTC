#include "hooks.hpp"
#include "../core/traffic_state.hpp"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <MinHook.h>
#include <Geode/Geode.hpp>

using namespace geode::prelude;

static void logConnect(const char* tag, const sockaddr* name) {
    if (!name) return;

    char ip[INET6_ADDRSTRLEN] = {};
    uint16_t port = 0;

    if (name->sa_family == AF_INET) {
        auto addr = reinterpret_cast<const sockaddr_in*>(name);
        inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
        port = ntohs(addr->sin_port);
    } else if (name->sa_family == AF_INET6) {
        auto addr = reinterpret_cast<const sockaddr_in6*>(name);
        inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
        port = ntohs(addr->sin6_port);
    } else {
        return;
    }

    auto line = fmt::format("[{}] {}:{}", tag, ip, port);
    log::info("[GTC] {}", line);
    gtc::core::TrafficState::instance().pushLog(line);
}

// --- connect() ---
using connect_t = int (WSAAPI*)(SOCKET, const sockaddr*, int);
static connect_t original_connect = nullptr;

static int WSAAPI hooked_connect(SOCKET s, const sockaddr* name, int namelen) {
    logConnect("connect", name);
    return original_connect(s, name, namelen);
}

// --- WSAConnect() ---
using WSAConnect_t = int (WSAAPI*)(SOCKET, const sockaddr*, int, LPWSABUF, LPWSABUF, LPQOS, LPQOS);
static WSAConnect_t original_WSAConnect = nullptr;

static int WSAAPI hooked_WSAConnect(
    SOCKET s, const sockaddr* name, int namelen,
    LPWSABUF caller, LPWSABUF callee, LPQOS sqos, LPQOS gqos
) {
    logConnect("WSAConnect", name);
    return original_WSAConnect(s, name, namelen, caller, callee, sqos, gqos);
}

namespace gtc::network {
    void installHooks() {
        if (MH_Initialize() != MH_OK) {
            log::error("[GTC] MinHook init failed");
            return;
        }

        HMODULE ws2 = GetModuleHandleA("ws2_32.dll");
        if (!ws2) {
            log::error("[GTC] ws2_32.dll not loaded");
            return;
        }

        auto connectTarget = reinterpret_cast<void*>(GetProcAddress(ws2, "connect"));
        if (!connectTarget || MH_CreateHook(connectTarget, reinterpret_cast<void*>(&hooked_connect),
                                             reinterpret_cast<void**>(&original_connect)) != MH_OK) {
            log::error("[GTC] Failed to hook connect");
        } else {
            MH_EnableHook(connectTarget);
            log::info("[GTC] connect() hooked");
        }

        auto wsaConnectTarget = reinterpret_cast<void*>(GetProcAddress(ws2, "WSAConnect"));
        if (!wsaConnectTarget || MH_CreateHook(wsaConnectTarget, reinterpret_cast<void*>(&hooked_WSAConnect),
                                                reinterpret_cast<void**>(&original_WSAConnect)) != MH_OK) {
            log::error("[GTC] Failed to hook WSAConnect");
        } else {
            MH_EnableHook(wsaConnectTarget);
            log::info("[GTC] WSAConnect() hooked");
        }

        log::info("[GTC] Network hooks installed");
    }

    void uninstallHooks() {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }
}
