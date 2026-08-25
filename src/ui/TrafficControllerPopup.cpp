#include "TrafficControllerPopup.hpp"
#include "WireGuardConfigPopup.hpp"
#include "VlessConfigPopup.hpp"

#include "../core/ping.hpp"
#include "../core/config_store.hpp"
#include "../network/wg_tunnel.hpp"

#include <thread>

using namespace gtc::core;

TrafficControllerPopup* TrafficControllerPopup::create() {
    auto ret = new TrafficControllerPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool TrafficControllerPopup::init() {
    if (!Popup::init(420.f, 320.f)) {
        return false;
    }

    this->setTitle("Traffic Controller");

    auto size = m_size;

    m_statusLabel = CCLabelBMFont::create("hooks active", "chatFont.fnt");
    m_statusLabel->setPosition({ size.width / 2.f, size.height - 45.f });
    m_statusLabel->setScale(0.6f);
    m_mainLayer->addChild(m_statusLabel);

    m_buttonMenu = CCMenu::create();
    m_buttonMenu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(m_buttonMenu);

    float rowY = size.height - 90.f;
    float rowStep = 40.f;

    for (int i = 0; i < 3; i++) {
        auto cat = static_cast<TrafficCategory>(i);
        auto status = TrafficState::instance().getStatus(cat);

        auto nameLabel = CCLabelBMFont::create(
            TrafficState::categoryName(cat),
            "goldFont.fnt"
        );
        nameLabel->setAnchorPoint({ 0.f, 0.5f });
        nameLabel->setPosition({ 20.f, rowY });
        nameLabel->setScale(0.5f);
        m_mainLayer->addChild(nameLabel);

        auto sprite = ButtonSprite::create(
            TrafficState::protocolName(status.protocol),
            "goldFont.fnt",
            "GJ_button_04.png",
            0.6f
        );

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(TrafficControllerPopup::onCycleProtocol)
        );

        button->setTag(i);
        button->setPosition({ 210.f, rowY });
        m_buttonMenu->addChild(button);

        auto pingLabel = CCLabelBMFont::create(
            status.pingText.c_str(),
            "chatFont.fnt"
        );
        pingLabel->setAnchorPoint({ 1.f, 0.5f });
        pingLabel->setPosition({ size.width - 20.f, rowY });
        pingLabel->setScale(0.5f);
        m_mainLayer->addChild(pingLabel);

        m_rows[i].protocolButton = button;
        m_rows[i].protocolSprite = sprite;
        m_rows[i].pingLabel = pingLabel;

        rowY -= rowStep;
    }

    m_logLabel = CCLabelBMFont::create("", "chatFont.fnt");
    m_logLabel->setAnchorPoint({ 0.f, 1.f });
    m_logLabel->setPosition({ 20.f, rowY - 10.f });
    m_logLabel->setScale(0.4f);
    m_mainLayer->addChild(m_logLabel);

    this->schedule(
        schedule_selector(TrafficControllerPopup::onRefresh),
        3.0f
    );

    this->onRefresh(0.f);

    auto configMenu = CCMenu::create();
    configMenu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(configMenu);

    auto wgSprite = ButtonSprite::create(
        "WG Config",
        "goldFont.fnt",
        "GJ_button_04.png",
        0.5f
    );

    auto wgButton = CCMenuItemSpriteExtra::create(
        wgSprite,
        this,
        menu_selector(TrafficControllerPopup::onOpenWgConfig)
    );

    wgButton->setPosition({ 80.f, 20.f });
    configMenu->addChild(wgButton);

    auto vlessSprite = ButtonSprite::create(
        "VLESS Config",
        "goldFont.fnt",
        "GJ_button_04.png",
        0.5f
    );

    auto vlessButton = CCMenuItemSpriteExtra::create(
        vlessSprite,
        this,
        menu_selector(TrafficControllerPopup::onOpenVlessConfig)
    );

    vlessButton->setPosition({ 210.f, 20.f });
    configMenu->addChild(vlessButton);

    auto testSprite = ButtonSprite::create(
        "Test Handshake",
        "goldFont.fnt",
        "GJ_button_04.png",
        0.5f
    );

    auto testButton = CCMenuItemSpriteExtra::create(
        testSprite,
        this,
        menu_selector(TrafficControllerPopup::onTestHandshake)
    );

    testButton->setPosition({ 350.f, 20.f });
    configMenu->addChild(testButton);

    return true;
}

void TrafficControllerPopup::onCycleProtocol(CCObject* sender) {
    auto button = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto cat = static_cast<TrafficCategory>(button->getTag());

    auto current = TrafficState::instance().getStatus(cat).protocol;
    auto next = TrafficState::nextProtocol(current);

    TrafficState::instance().setProtocol(cat, next);

    updateRow(cat);
}

void TrafficControllerPopup::updateRow(TrafficCategory cat) {
    int i = static_cast<int>(cat);
    auto status = TrafficState::instance().getStatus(cat);

    m_rows[i].protocolSprite->setString(
        TrafficState::protocolName(status.protocol)
    );

    m_rows[i].pingLabel->setString(
        status.pingText.c_str()
    );
}

void TrafficControllerPopup::spawnPingCheck(TrafficCategory cat) {
    if (TrafficState::instance().getStatus(cat).checking) {
        return;
    }

    TrafficState::instance().setChecking(cat, true);

    std::thread([cat]() {
        std::string text;

        switch (cat) {
            case TrafficCategory::GeodeModsUpdate: {
                int ms = measureTcpPingMs(
                    "api.geode-sdk.org",
                    443
                );

                text = ms >= 0
                    ? fmt::format("{} ms", ms)
                    : "N/A";

                break;
            }

            case TrafficCategory::GdMusicDownload: {
                int ngMs = measureTcpPingMs(
                    "www.newgrounds.com",
                    443
                );

                int audioMs = measureTcpPingMs(
                    "audio.ngfiles.com",
                    443
                );

                text = fmt::format(
                    "NG:{} Audio:{}",
                    ngMs >= 0
                        ? std::to_string(ngMs) + "ms"
                        : "N/A",

                    audioMs >= 0
                        ? std::to_string(audioMs) + "ms"
                        : "N/A"
                );

                break;
            }

            case TrafficCategory::GlobedOnline: {
                int ms = measureTcpPingMs(
                    "138.201.21.106",
                    443
                );

                text = ms >= 0
                    ? fmt::format("{} ms", ms)
                    : "N/A (UDP relay)";

                break;
            }

            default:
                break;
        }

        TrafficState::instance().setPingText(cat, text);
        TrafficState::instance().setChecking(cat, false);
    }).detach();
}

void TrafficControllerPopup::updateLog() {
    auto snapshot = TrafficState::instance().getLogSnapshot();

    std::string text;
    int count = 0;

    for (
        auto it = snapshot.rbegin();
        it != snapshot.rend() && count < 6;
        ++it, ++count
    ) {
        text = *it + "\n" + text;
    }

    m_logLabel->setString(text.c_str());
}

void TrafficControllerPopup::onRefresh(float dt) {
    for (int i = 0; i < 3; i++) {
        auto cat = static_cast<TrafficCategory>(i);

        spawnPingCheck(cat);
        updateRow(cat);
    }

    updateLog();
}

void TrafficControllerPopup::onOpenWgConfig(CCObject*) {
    WireGuardConfigPopup::create()->show();
}

void TrafficControllerPopup::onOpenVlessConfig(CCObject*) {
    VlessConfigPopup::create()->show();
}

void TrafficControllerPopup::onTestHandshake(CCObject*) {
    auto cfg = loadWireGuardConfig();
    if (!cfg.isValid()) {
        TrafficState::instance().pushLog("[WG Test] Config incomplete");
        return;
    }

    TrafficState::instance().pushLog("[WG Test] Starting handshake test...");

    std::thread([cfg]() {
        auto result = gtc::network::testWireGuardHandshake(cfg);

        if (result.success) {
            TrafficState::instance().pushLog(fmt::format(
                "[WG Test] SUCCESS - age: {}ms, rtt: {}ms, loss: {:.1f}%",
                result.timeSinceHandshakeMs,
                result.estimatedRttMs,
                result.estimatedLoss * 100.f
            ));
        } else {
            TrafficState::instance().pushLog(fmt::format(
                "[WG Test] FAILED - {}", result.message
            ));
        }
    }).detach();
}
