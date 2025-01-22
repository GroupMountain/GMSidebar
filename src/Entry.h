#pragma once
#include "Config.h"
#include "Global.h"
#include <ll/api/mod/NativeMod.h>
#include <ll/api/mod/RegisterHelper.h>

namespace gmsidebar {

class Entry {

public:
    static Entry& getInstance();

    Entry() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    /// @return True if the mod is loaded successfully.
    bool load();

    /// @return True if the mod is enabled successfully.
    bool enable();

    /// @return True if the mod is disabled successfully.
    bool disable();

    /// @return True if the mod is unloaded successfully.
    bool unload();

    Config& getConfig() { return mConfig.value(); }

    std::unique_ptr<GMLIB::Files::I18n::LangI18n>& getI18n() { return mI18n; }

    std::string translate(std::string key, std::vector<std::string> data = {}, std::string translateKey = "%0$s") {
        return mI18n->translate(key, data, translateKey);
    }

private:
    ll::mod::NativeMod&                           mSelf;
    std::optional<Config>                         mConfig;
    std::unique_ptr<GMLIB::Files::I18n::LangI18n> mI18n;
};

} // namespace gmsidebar