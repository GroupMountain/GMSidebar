#pragma once
#include "Global.h"
#include <unordered_map>

namespace gmsidebar {
struct Config {
    int version = 1;

    std::string language = "zh_CN";

    ObjectiveSortOrder sortType = ObjectiveSortOrder::Ascending;

    struct TitleInfo {
        std::vector<std::string> data           = {"§aGMSidebar", "§bGMSidebar", "§cGMSidebar"};
        int                      updateInverval = 2;
    } title;

    struct DataInfo {
        std::vector<std::string> data;
        int                      updateInverval;
    };

    std::unordered_map<std::string, DataInfo> sidebarInfo = {
        {"1", DataInfo{{"§aConfig Example", "§bConfig Example", "§cConfig Example"}, 1} },
        {"2", DataInfo{{"Name: %player_realname%", "Ping: %player_ping%ms"}, 2}                },
        {"3", DataInfo{{"Mspt: %server_mspt_colored%", "Tps: %server_tps_colored%"}, 3}}
    };
};
} // namespace gmsidebar