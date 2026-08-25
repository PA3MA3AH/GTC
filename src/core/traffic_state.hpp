#pragma once
#include <string>
#include <deque>
#include <array>
#include <mutex>
#include <chrono>

namespace gtc::core {

enum class TrafficCategory {
    GlobedOnline = 0,
    GeodeModsUpdate = 1,
    GdMusicDownload = 2,
    Count = 3
};

enum class Protocol {
    WireGuard = 0,
    VLESS = 1
};

struct CategoryStatus {
    Protocol protocol = Protocol::WireGuard;
    std::string pingText = "...";
    bool checking = false;
};

class TrafficState {
public:
    static TrafficState& instance() {
        static TrafficState inst;
        return inst;
    }

    void pushLog(const std::string& line) {
        std::lock_guard lock(m_logMutex);
        m_log.push_back(line);
        if (m_log.size() > 200) m_log.pop_front();
    }

    std::deque<std::string> getLogSnapshot() {
        std::lock_guard lock(m_logMutex);
        return m_log;
    }

    CategoryStatus getStatus(TrafficCategory cat) {
        std::lock_guard lock(m_statusMutex);
        return m_status[static_cast<size_t>(cat)];
    }

    void setProtocol(TrafficCategory cat, Protocol p) {
        std::lock_guard lock(m_statusMutex);
        m_status[static_cast<size_t>(cat)].protocol = p;
    }

    void setPingText(TrafficCategory cat, const std::string& text) {
        std::lock_guard lock(m_statusMutex);
        m_status[static_cast<size_t>(cat)].pingText = text;
    }

    void setChecking(TrafficCategory cat, bool v) {
        std::lock_guard lock(m_statusMutex);
        m_status[static_cast<size_t>(cat)].checking = v;
    }

    static const char* categoryName(TrafficCategory cat) {
        switch (cat) {
            case TrafficCategory::GlobedOnline:     return "Globed Online";
            case TrafficCategory::GeodeModsUpdate:  return "Geode Mods Update";
            case TrafficCategory::GdMusicDownload:  return "GD Music Download";
            default: return "?";
        }
    }

    static const char* protocolName(Protocol p) {
        return p == Protocol::WireGuard ? "WireGuard" : "VLESS";
    }

    static Protocol nextProtocol(Protocol p) {
        return p == Protocol::WireGuard ? Protocol::VLESS : Protocol::WireGuard;
    }

private:
    TrafficState() = default;
    std::mutex m_logMutex;
    std::mutex m_statusMutex;
    std::deque<std::string> m_log;
    std::array<CategoryStatus, static_cast<size_t>(TrafficCategory::Count)> m_status;
};

}
