#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "TrafficControllerPopup.hpp"

using namespace geode::prelude;

class $modify(TrafficMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto button = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_likeBtn_001.png"),
            this,
            menu_selector(TrafficMenuLayer::onTrafficController)
        );
        button->setID("traffic-controller-button"_spr);

        auto menu = this->getChildByID("bottom-menu");
        if (menu) {
            menu->addChild(button);
            menu->updateLayout();
        }

        log::info("[GTC] TrafficMenuLayer button added");
        return true;
    }

    void onTrafficController(CCObject*) {
        TrafficControllerPopup::create()->show();
    }
};
