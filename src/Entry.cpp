#include "Entry.h"
#include "Global.h"
#include "Language.h"
#include "ll/api/Config.h"

ll::Logger logger(PLUGIN_NAME);

namespace gmsidebar {

std::unique_ptr<Entry>& Entry::getInstance() {
    static std::unique_ptr<Entry> instance;
    return instance;
}

bool Entry::load() {
    // Code for loading the plugin goes here.
    return true;
}

bool Entry::enable() {
    // Code for enabling the plugin goes here.
    mConfig.emplace();
    ll::config::loadConfig(*mConfig, getSelf().getConfigDir() / u8"config.json");
    ll::config::saveConfig(*mConfig, getSelf().getConfigDir() / u8"config.json");
    mI18n = std::make_unique<GMLIB::Files::I18n::LangI18n>(getSelf().getLangDir());
    mI18n->updateOrCreateLanguage("zh_CN", zh_CN);
    mI18n->loadAllLanguages();
    mI18n->chooseLanguage(mConfig->language);
    mI18n->setDefaultLanguage("zh_CN");
    loadSidebarStatus();
    init();
    registerCommand();
    logger.info("GMSidebar Loaded!");
    logger.info("Author: GroupMountain");
    logger.info("Repository: https://github.com/GroupMountain/GMSidebar");
    return true;
}

bool Entry::disable() {
    // Code for disabling the plugin goes here.
    saveSidebarStatus();
    mConfig.reset();
    mI18n.reset();
    disablePlugin();
    return true;
}

bool Entry::unload() {
    // Code for disabling the plugin goes here.
    return true;
}

} // namespace gmsidebar

LL_REGISTER_PLUGIN(gmsidebar::Entry, gmsidebar::Entry::getInstance());