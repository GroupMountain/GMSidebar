#pragma once
// #include <include_all.h>
//#include "GMLIB/Server/PlayerAPI.h"
#include "gmlib/include_all.h"
#include "mc/world/scores/ObjectiveSortOrder.h"


#define MOD_NAME "GMSidebar"

extern void init();
extern void disableMod();

extern void loadSidebarStatus();
extern void saveSidebarStatus();

extern void registerCommand();

extern std::string tr(std::string key, std::vector<std::string> data = {}, std::string translateKey = "%0$s");