#include "gmsidebar/Entry.h"
#include "gmsidebar/GMSidebar.h"
#include <ll/api/i18n/I18n.h>
#include <ll/api/mod/RegisterHelper.h>

namespace gmsidebar {

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

bool Entry::load() {
    if (auto result = ll::i18n::getInstance().load(getSelf().getLangDir()); !result) {
        getLogger().error("Failed to load i18n");
        result.error().log(getLogger());
    }
    return true;
}

bool Entry::enable() {
    GMSidebar::getInstance().enable();
    registerCmd();
    exportApi();
    return true;
}

bool Entry::disable() {
    GMSidebar::getInstance().disable();
    removeApi();
    return true;
}

bool Entry::unload() { return true; }

} // namespace gmsidebar

LL_REGISTER_MOD(gmsidebar::Entry, gmsidebar::Entry::getInstance());