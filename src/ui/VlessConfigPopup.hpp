#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

using namespace geode::prelude;

class VlessConfigPopup : public geode::Popup {
public:
    static VlessConfigPopup* create();

protected:
    bool init();
    void onSave(CCObject*);

private:
    TextInput* m_uriInput = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
};
