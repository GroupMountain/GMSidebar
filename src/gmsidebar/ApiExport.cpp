#include "gmsidebar/Entry.h"
#include "gmsidebar/GMSidebar.h"
#include <LegacyRemoteCall/RemoteCallAPI.h>

namespace gmsidebar {

void Entry::exportApi() {
    LegacyRemoteCall::exportAs("GMSidebar", "isEnable", []() { return GMSidebar::getInstance().isEnabled(); });
    LegacyRemoteCall::exportAs("GMSidebar", "enable", []() { GMSidebar::getInstance().enable(); });
    LegacyRemoteCall::exportAs("GMSidebar", "disable", []() { GMSidebar::getInstance().disable(); });
    LegacyRemoteCall::exportAs("GMSidebar", "loadConfig", []() {
        GMSidebar::getInstance().loadConfig(std::nullopt);
    });
    LegacyRemoteCall::exportAs("GMSidebar", "saveConfig", []() {
        GMSidebar::getInstance().saveConfig(std::nullopt);
    });
    LegacyRemoteCall::exportAs("GMSidebar", "loadConfigFromPath", [](std::string const& path) {
        GMSidebar::getInstance().loadConfig(path);
    });
    LegacyRemoteCall::exportAs("GMSidebar", "saveConfigFromPath", [](std::string const& path) {
        GMSidebar::getInstance().saveConfig(path);
    });
    LegacyRemoteCall::exportAs("GMSidebar", "isPlayerSidebarEnabled", [](std::string const& uuid) {
        return GMSidebar::getInstance().isPlayerSidebarEnabled(mce::UUID::fromString(uuid));
    });
    LegacyRemoteCall::exportAs("GMSidebar", "setPlayerSidebarEnabled", [](std::string const& uuid, bool enabled) {
        return GMSidebar::getInstance().setPlayerSidebarEnabled(mce::UUID::fromString(uuid), enabled);
    });
    LegacyRemoteCall::exportAs("GMSidebar", "clearPlayerCache", [](std::string const& uuid) {
        return GMSidebar::getInstance().clearPlayerCache(mce::UUID::fromString(uuid));
    });
    LegacyRemoteCall::exportAs("GMSidebar", "clearAllPlayerCache", []() {
        GMSidebar::getInstance().clearAllPlayerCache();
    });
}

} // namespace gmsidebar