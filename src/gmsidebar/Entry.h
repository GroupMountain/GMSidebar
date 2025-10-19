#pragma once
#include <ll/api/mod/NativeMod.h>

namespace gmsidebar {

class Entry {

public:
    static Entry& getInstance();

    Entry() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }
    [[nodiscard]] ll::io::Logger&     getLogger() const { return getSelf().getLogger(); }

    bool load();
    bool enable();
    bool disable();
    bool unload();

    void registerCmd();
    void exportApi();
    void removeApi();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace gmsidebar