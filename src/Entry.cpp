#include "Entry.h"
#include "Global.h"

ll::Logger logger(PLUGIN_NAME);

namespace my_plugin {

std::unique_ptr<MyPlugin>& MyPlugin::getInstance() {
    static std::unique_ptr<MyPlugin> instance;
    return instance;
}

bool MyPlugin::load() {
    // Code for loading the plugin goes here.
    return true;
}

bool MyPlugin::enable() {
    // Code for enabling the plugin goes here.
    return true;
}

bool MyPlugin::disable() {
    // Code for disabling the plugin goes here.
    return true;
}

} // namespace my_plugin

LL_REGISTER_PLUGIN(my_plugin::MyPlugin, my_plugin::MyPlugin::getInstance());