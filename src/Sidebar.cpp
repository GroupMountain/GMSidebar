#include "Entry.h"
#include "GMSidebarAPI.h"
#include "Global.h"

std::string                          mTitle;
std::unordered_map<int, std::string> mDataMap;
std::unordered_map<int, int>         mDataIndex;
ObjectiveSortOrder                   mOrder;
int                                  mTitleIndex = 0;

std::unordered_map<mce::UUID, bool> mPlayerSidebarStatus;
ll::thread::ThreadPoolExecutor      mThreadPool = ll::thread::ThreadPoolExecutor{"GMSidebar"};

using namespace ll::chrono_literals;

std::string tr(std::string key, std::vector<std::string> data, std::string translateKey) {
    return gmsidebar::Entry::getInstance().translate(key, data, translateKey);
}

void saveSidebarStatus() {
    nlohmann::json json;
    for (auto& [key, val] : mPlayerSidebarStatus) {
        json[key.asString()] = val;
    }
    GMLIB::Files::JsonFile::writeFile("./plugins/GMSidebar/data/PlayerStatus.json", json);
}

void loadSidebarStatus() {
    auto json =
        GMLIB::Files::JsonFile::initJson("./plugins/GMSidebar/data/PlayerStatus.json", nlohmann::json::object());
    for (nlohmann::json::const_iterator it = json.begin(); it != json.end(); ++it) {
        if (it.value().is_boolean()) {
            auto uuid                  = mce::UUID::fromString(it.key());
            mPlayerSidebarStatus[uuid] = it.value().get<bool>();
        }
    }
}

void sendSidebar(GMLIB_Player* pl) {
    std::vector<std::pair<std::string, int>> dataList;
    for (auto& [mapIndex, mapData] : mDataMap) {
        dataList.push_back({mapData, mapIndex});
    }
    for (auto& [data, index] : dataList) {
        GMLIB::Server::PlaceholderAPI::translate(data, pl);
    }
    auto title = GMLIB::Server::PlaceholderAPI::translateString(mTitle, pl);
    pl->setClientSidebar(title, dataList, mOrder);
}

void sendSidebarToClients() {
    GMLIB_Level::getInstance()->forEachPlayer([&](Player& player) -> bool {
        if (player.isSimulatedPlayer()) return true;
        auto pl = (GMLIB_Player*)&player;
        pl->removeClientSidebar();
        if (!mPlayerSidebarStatus.contains(pl->getUuid())) {
            mPlayerSidebarStatus[pl->getUuid()] = true;
            saveSidebarStatus();
        }
        if (mPlayerSidebarStatus[pl->getUuid()]) {
            sendSidebar(pl);
        }
        return true;
    });
}

void init() {
    auto& config = gmsidebar::Entry::getInstance().getConfig();
    mOrder       = config.sortType;
    for (auto& [index, info] : config.sidebarInfo) {
        try {
            auto num = std::stoi(index);
            if (info.updateInverval > 0) {
                ll::coro::keepThis([=]() -> ll::coro::CoroTask<> {
                    while (true) {
                        co_await std::chrono::seconds::duration(info.updateInverval);
                        mDataMap[num] = info.data[mDataIndex[num]];
                        mDataIndex[num]++;
                        if (mDataIndex[num] >= info.data.size()) mDataIndex[num] = 0;
                    }
                    co_return;
                }).launch(mThreadPool);
            } else {
                mDataMap[num] = info.data[0];
            }
            GMLIB::Server::PlaceholderAPI::translate(mDataMap[num]);
        } catch (...) {}
    }
    if (config.title.updateInverval > 0) {
        ll::coro::keepThis([=]() -> ll::coro::CoroTask<> {
            while (true) {
                co_await std::chrono::seconds::duration(config.title.updateInverval);
                mTitle = config.title.data[mTitleIndex];
                mTitleIndex++;
                if (mTitleIndex >= config.title.data.size()) mTitleIndex = 0;
            }
            co_return;
        }).launch(mThreadPool);
    } else {
        mTitle = config.title.data[0];
    }
    ll::coro::keepThis([=]() -> ll::coro::CoroTask<> {
        while (true) {
            co_await 1s;
            sendSidebarToClients();
        }
        co_return;
    }).launch(ll::thread::ServerThreadExecutor::getDefault());
}

void disableMod() {
    mThreadPool.destroy();
    GMLIB_Level::getInstance()->forEachPlayer([&](Player& player) -> bool {
        auto pl = (GMLIB_Player*)&player;
        pl->removeClientSidebar();
        return true;
    });
}

void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance()
                    .getOrCreateCommand("sidebar", tr("sidebar.command.desc"), CommandPermissionLevel::Any);
    cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
        auto entity = (GMLIB_Actor*)origin.getEntity();
        if (entity && entity->isPlayer()) {
            auto pl                             = (Player*)entity;
            auto res                            = !mPlayerSidebarStatus[pl->getUuid()];
            mPlayerSidebarStatus[pl->getUuid()] = res;
            saveSidebarStatus();
            return output.success(res ? tr("sidebar.command.toggle.on") : tr("sidebar.command.toggle.off"));
        }
        return output.error(tr("sidebar.command.console"));
    });
}

namespace GMSidebar {

bool isPlayerSidebarEnabled(mce::UUID const& uuid) { return mPlayerSidebarStatus[uuid]; }

void setPlayerSidebarEnabled(mce::UUID const& uuid, bool setting) {
    mPlayerSidebarStatus[uuid] = setting;
    saveSidebarStatus();
}

} // namespace GMSidebar
