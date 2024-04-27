#pragma once
#include <include_all.h>

#define PLUGIN_NAME "GMSidebar"

extern ll::Logger logger;

extern void init();
extern void disablePlugin();

extern void loadSidebarStatus();
extern void saveSidebarStatus();

extern void registerCommand();

extern std::string tr(std::string key, std::vector<std::string> data = {}, std::string translateKey = "%0$s");