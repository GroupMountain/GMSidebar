#pragma once
#include "Config.h"
#include "Global.h"
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/RegisterHelper.h>

namespace gmsidebar {

class Entry {

public:
    static std::unique_ptr<Entry>& getInstance();

    Entry(ll::plugin::NativePlugin& self) : mSelf(self) {}

    [[nodiscard]] ll::plugin::NativePlugin& getSelf() const { return mSelf; }

    /// @return True if the plugin is loaded successfully.
    bool load();

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();

    /// @return True if the plugin is unloaded successfully.
    bool unload();

    Config& getConfig() { return mConfig.value(); }

    std::unique_ptr<GMLIB::Files::I18n::LangI18n>& getI18n() { return mI18n; }

    std::string translate(std::string key, std::vector<std::string> data = {}, std::string translateKey = "%0$s") {
        return mI18n->translate(key, data, translateKey);
    }

private:
    ll::plugin::NativePlugin&                     mSelf;
    std::optional<Config>                         mConfig;
    std::unique_ptr<GMLIB::Files::I18n::LangI18n> mI18n;
};

} // namespace gmsidebar