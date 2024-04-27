#include "Global.h"
#include <unordered_map>

struct Config {
    int version = 1;

    ObjectiveSortOrder sortType = ObjectiveSortOrder::Ascending;

    struct TitleInfo {
        std::vector<std::string> data = {"§aGMSidebar", "§bGMSidebar", "§cGMSidebar"};
        int                      updateInverval =2;
    } title;

    struct DataInfo {
        std::vector<std::string> data;
        int                      updateInverval;
    };

    std::unordered_map<std::string, DataInfo> sidebarInfo = {
        {"1", DataInfo{{"§aConfig Example", "§bConfig Example", "§cConfig Example"}, 1}},
        {"2", DataInfo{{"Player Name: %player_name%", "Player Ping: %player_ping%ms"}, 2} },
        {"3", DataInfo{{"Server Mspt: %server_mspt%", "Server Tps: %server_tps%"}, 3}     }
    };
};