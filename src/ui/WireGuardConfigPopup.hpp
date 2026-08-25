#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class WireGuardConfigPopup : public geode::Popup {
public:
    static WireGuardConfigPopup* create();

protected:
    bool init();
    void onSave(CCObject*);

private:
    TextInput* m_privateKeyInput = nullptr;
    TextInput* m_addressInput = nullptr;
    TextInput* m_peerPublicKeyInput = nullptr;
    TextInput* m_endpointInput = nullptr;
    TextInput* m_allowedIpsInput = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
};
