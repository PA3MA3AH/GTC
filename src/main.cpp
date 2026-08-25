#include <Geode/Geode.hpp>
#include "network/hooks.hpp"
#include "network/wg_smoketest.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("[GTC] loading...");
    gtc::network::installHooks();
    gtc::network::runWireGuardSmokeTest();
}
