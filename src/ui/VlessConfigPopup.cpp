#include "VlessConfigPopup.hpp"
#include "../core/vless_config.hpp"
#include "../core/config_store.hpp"

using namespace gtc::core;

VlessConfigPopup* VlessConfigPopup::create() {
    auto ret = new VlessConfigPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool VlessConfigPopup::init() {
    if (!Popup::init(420.f, 180.f)) {
        return false;
    }

    this->setTitle("VLESS Config");

    auto size = m_size;

    auto label = CCLabelBMFont::create("Paste vless:// link:", "goldFont.fnt");
    label->setPosition({ size.width / 2.f, size.height - 60.f });
    label->setScale(0.5f);
    m_mainLayer->addChild(label);

    auto existing = loadVlessConfig();
    m_uriInput = TextInput::create(360.f, "vless://uuid@host:port?...");
    m_uriInput->setPosition({ size.width / 2.f, size.height - 90.f });
    m_uriInput->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+/=:._-@?&#%");
    if (existing) {
        m_uriInput->setString(fmt::format(
            "vless://{}@{}:{}?security={}&sni={}&pbk={}&sid={}#{}",
            existing->uuid, existing->host, existing->port,
            existing->security, existing->sni, existing->publicKey, existing->shortId, existing->remark
        ));
    }
    m_mainLayer->addChild(m_uriInput);

    m_statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
    m_statusLabel->setPosition({ size.width / 2.f, 40.f });
    m_statusLabel->setScale(0.5f);
    m_mainLayer->addChild(m_statusLabel);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(menu);

    auto saveSprite = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    auto saveButton = CCMenuItemSpriteExtra::create(saveSprite, this, menu_selector(VlessConfigPopup::onSave));
    saveButton->setPosition({ size.width / 2.f, 15.f });
    menu->addChild(saveButton);

    return true;
}

void VlessConfigPopup::onSave(CCObject*) {
    auto parsed = VlessConfig::parse(m_uriInput->getString());
    if (!parsed || !parsed->isValid()) {
        m_statusLabel->setString("Invalid vless:// URI");
        return;
    }

    saveVlessConfig(*parsed);
    m_statusLabel->setString("Saved!");
}
