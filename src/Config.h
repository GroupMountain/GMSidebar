#pragma once
#include "Global.h"
#include <unordered_map>

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
        {"2", DataInfo{{"Name: %player_name%", "Ping: %player_ping%ms"}, 2}                },
        {"3", DataInfo{{"Mspt: %server_mspt_colored_2%", "Tps: %server_tps_colored_2%"}, 3}},
        {"4", DataInfo{{"Score1: %player_score_test1%", "Score2: %player_score_test2%"}, 3}}
    };
};