#include "Entry.h"
#include "Global.h"
#include "ll/api/Config.h"

ll::Logger logger(PLUGIN_NAME);

namespace my_plugin {

std::unique_ptr<MyPlugin>& MyPlugin::getInstance() {
    static std::unique_ptr<MyPlugin> instance;
    return instance;
}

bool MyPlugin::load() {
    // Code for loading the plugin goes here.
    mConfig.emplace();
    ll::config::loadConfig(*mConfig, getSelf().getConfigDir() / u8"config.json");
    ll::config::saveConfig(*mConfig, getSelf().getConfigDir() / u8"config.json");
    return true;
}

bool MyPlugin::enable() {
    // Code for enabling the plugin goes here.
    init();
    return true;
}

bool MyPlugin::disable() {
    // Code for disabling the plugin goes here.
    return true;
}

} // namespace my_plugin

LL_REGISTER_PLUGIN(my_plugin::MyPlugin, my_plugin::MyPlugin::getInstance());