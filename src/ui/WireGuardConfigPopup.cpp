#include "WireGuardConfigPopup.hpp"
#include "../core/wg_config.hpp"
#include "../core/config_store.hpp"

using namespace gtc::core;

WireGuardConfigPopup* WireGuardConfigPopup::create() {
    auto ret = new WireGuardConfigPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

static TextInput* makeField(CCLayer* parent, float y, float width, const char* placeholder, const std::string& existing) {
    auto input = TextInput::create(width, placeholder);
    input->setPosition({ 210.f, y });
    input->setFilter("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789+/=:._-@?&#%");
    if (!existing.empty()) input->setString(existing);
    parent->addChild(input);
    return input;
}

bool WireGuardConfigPopup::init() {
    if (!Popup::init(420.f, 320.f)) {
        return false;
    }

    this->setTitle("WireGuard Config");

    auto existing = loadWireGuardConfig();
    auto size = m_size;
    float y = size.height - 60.f;
    float step = 45.f;

    auto addLabel = [&](const char* text, float posY) {
        auto label = CCLabelBMFont::create(text, "goldFont.fnt");
        label->setAnchorPoint({ 0.f, 0.5f });
        label->setPosition({ 20.f, posY });
        label->setScale(0.5f);
        m_mainLayer->addChild(label);
    };

    addLabel("Private Key", y);
    m_privateKeyInput = makeField(m_mainLayer, y, 180.f, "base64 private key", existing.privateKey);
    y -= step;

    addLabel("Address", y);
    m_addressInput = makeField(m_mainLayer, y, 180.f, "10.66.66.x/24", existing.address);
    y -= step;

    addLabel("Peer Public Key", y);
    m_peerPublicKeyInput = makeField(m_mainLayer, y, 180.f, "base64 public key", existing.peerPublicKey);
    y -= step;

    addLabel("Endpoint", y);
    m_endpointInput = makeField(m_mainLayer, y, 180.f, "host:port", existing.endpoint);
    y -= step;

    addLabel("Allowed IPs", y);
    m_allowedIpsInput = makeField(m_mainLayer, y, 180.f, "0.0.0.0/0", existing.allowedIps);
    y -= step;

    m_statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
    m_statusLabel->setPosition({ size.width / 2.f, 40.f });
    m_statusLabel->setScale(0.5f);
    m_mainLayer->addChild(m_statusLabel);

    auto menu = CCMenu::create();
    menu->setPosition({ 0.f, 0.f });
    m_mainLayer->addChild(menu);

    auto saveSprite = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_01.png", 0.8f);
    auto saveButton = CCMenuItemSpriteExtra::create(saveSprite, this, menu_selector(WireGuardConfigPopup::onSave));
    saveButton->setPosition({ size.width / 2.f, 15.f });
    menu->addChild(saveButton);

    return true;
}

void WireGuardConfigPopup::onSave(CCObject*) {
    WireGuardConfig cfg;
    cfg.privateKey = m_privateKeyInput->getString();
    cfg.address = m_addressInput->getString();
    cfg.peerPublicKey = m_peerPublicKeyInput->getString();
    cfg.endpoint = m_endpointInput->getString();
    cfg.allowedIps = m_allowedIpsInput->getString();

    if (!cfg.isValid()) {
        m_statusLabel->setString("Missing required fields");
        return;
    }

    saveWireGuardConfig(cfg);
    m_statusLabel->setString("Saved!");
}
