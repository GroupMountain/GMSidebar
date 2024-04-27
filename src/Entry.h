#pragma once
#include "Config.h"
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/RegisterHelper.h>

namespace my_plugin {

class MyPlugin {

public:
    static std::unique_ptr<MyPlugin>& getInstance();

    MyPlugin(ll::plugin::NativePlugin& self) : mSelf(self) {}

    [[nodiscard]] ll::plugin::NativePlugin& getSelf() const { return mSelf; }

    /// @return True if the plugin is loaded successfully.
    bool load();

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();

    // TODO: Implement this method if you need to unload the plugin.
    // /// @return True if the plugin is unloaded successfully.
    // bool unload();

    Config& getConfig() { return mConfig.value(); }

private:
    ll::plugin::NativePlugin& mSelf;
    std::optional<Config>     mConfig;
};

} // namespace my_plugin