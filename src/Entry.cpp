#include "Entry.h"
#include "GMSidebarAPI.h"
#include "Global.h"
#include "resource.h"
#include "legacyRemoteCall/RemoteCallAPI.h"

namespace gmsidebar {

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

bool Entry::load() {
    // Code for loading the mod goes here.
    return true;
}

bool Entry::enable() {
    // Code for enabling the mod goes here.
    mConfig.emplace();
    if (!ll::config::loadConfig(*mConfig, getSelf().getConfigDir() / u8"config.json")) {
        ll::config::saveConfig(*mConfig, getSelf().getConfigDir() / u8"config.json");
    }
    mI18n.emplace(getSelf().getLangDir(), "zh_CN");
    mI18n->updateOrCreateLanguage("zh_CN", zh_CN);
    mI18n->loadAllLanguages();
    mI18n->chooseLanguage(mConfig->language);
    loadSidebarStatus();
    init();
    registerCommand();
    getSelf().getLogger().info("GMSidebar Loaded!");
    getSelf().getLogger().info("Author: GroupMountain");
    getSelf().getLogger().info("Repository: https://github.com/GroupMountain/GMSidebar");
    LegacyRemoteCall::exportAs("GMSidebar", "isPlayerSidebarEnabled", [](std::string const& uuid) -> bool {
        auto uid = mce::UUID::fromString(uuid);
        return GMSidebar::isPlayerSidebarEnabled(uid);
    });
    LegacyRemoteCall::exportAs(
        "GMSidebar",
        "setPlayerSidebarEnabled",
        [](std::string const& uuid, bool enable) -> void {
            auto uid = mce::UUID::fromString(uuid);
            GMSidebar::setPlayerSidebarEnabled(uid, enable);
        }
    );
    return true;
}

bool Entry::disable() {
    // Code for disabling the mod goes here.
    saveSidebarStatus();
    mConfig.reset();
    mI18n.reset();
    disableMod();
    return true;
}

bool Entry::unload() {
    // Code for disabling the mod goes here.
    return true;
}

} // namespace gmsidebar

LL_REGISTER_MOD(gmsidebar::Entry, gmsidebar::Entry::getInstance());