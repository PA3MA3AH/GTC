#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../core/traffic_state.hpp"

using namespace geode::prelude;

class TrafficControllerPopup : public geode::Popup {
public:
    static TrafficControllerPopup* create();

protected:
    bool init();
    void onRefresh(float dt);
    void onCycleProtocol(CCObject* sender);
    void updateRow(gtc::core::TrafficCategory cat);
    void updateLog();
    void spawnPingCheck(gtc::core::TrafficCategory cat);
    void onOpenWgConfig(CCObject*);
    void onOpenVlessConfig(CCObject*);
    void onTestHandshake(CCObject*);

private:
    CCLabelBMFont* m_statusLabel = nullptr;

    struct RowUI {
        CCMenuItemSpriteExtra* protocolButton = nullptr;
        ButtonSprite* protocolSprite = nullptr;
        CCLabelBMFont* pingLabel = nullptr;
    };
    RowUI m_rows[3];

    CCLabelBMFont* m_logLabel = nullptr;
    CCMenu* m_buttonMenu = nullptr;
};
