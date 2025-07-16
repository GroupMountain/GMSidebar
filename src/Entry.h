#pragma once
#include "Config.h"
#include "Global.h"

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

    gmlib::i18n::LangI18n& getI18n() { return *mI18n; }

private:
    ll::mod::NativeMod&                  mSelf;
    std::optional<Config>                mConfig;
    std::optional<gmlib::i18n::LangI18n> mI18n;
};

} // namespace gmsidebar

GMLIB_LANGI18N_LITERALS(gmsidebar::Entry::getInstance().getI18n())