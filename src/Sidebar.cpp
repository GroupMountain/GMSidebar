#include "Entry.h"
#include "GMSidebarAPI.h"
#include "Global.h"
#include "gmlib/gm/papi/PlaceholderAPI.h"
#include "gmlib/mc/world/Level.h"
#include "gmlib/mc/world/actor/Actor.h"
#include "gmlib/mc/world/actor/Player.h"

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
    gmlib::utils::JsonUtils::writeFile("./plugins/GMSidebar/data/PlayerStatus.json", json);
}

void loadSidebarStatus() {
    auto json =
        gmlib::utils::JsonUtils::initJson("./plugins/GMSidebar/data/PlayerStatus.json", nlohmann::json::object());
    for (nlohmann::json::const_iterator it = json.begin(); it != json.end(); ++it) {
        if (it.value().is_boolean()) {
            auto uuid                  = mce::UUID::fromString(it.key());
            mPlayerSidebarStatus[uuid] = it.value().get<bool>();
        }
    }
}

void sendSidebar(gmlib::world::actor::GMPlayer* pl) {
    std::vector<std::pair<std::string, int>> dataList;
    for (auto& [mapIndex, mapData] : mDataMap) {
        dataList.push_back({mapData, mapIndex});
    }
    for (auto& [data, index] : dataList) {
        gmlib::papi::PlaceholderAPI::translate(data, (gmlib::world::actor::GMActor*)pl);
    }
    auto title = gmlib::papi::PlaceholderAPI::translate(mTitle, (gmlib::world::actor::GMActor*)pl);
    pl->setClientSidebar(title, dataList, mOrder);
}

void sendSidebarToClients() {
    ll::service::getLevel()->forEachPlayer([&](Player& player) -> bool {
        if (player.isSimulatedPlayer()) return true;
        auto pl = (gmlib::world::actor::GMPlayer*)&player;
        if (!mPlayerSidebarStatus.contains(pl->getUuid())) {
            mPlayerSidebarStatus[pl->getUuid()] = true;
            saveSidebarStatus();
        }
        if (mPlayerSidebarStatus[pl->getUuid()]) {
            pl->removeClientSidebar();
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
                        co_await std::chrono::seconds{info.updateInverval};
                        mDataMap[num] = info.data[mDataIndex[num]];
                        mDataIndex[num]++;
                        if (mDataIndex[num] >= info.data.size()) mDataIndex[num] = 0;
                    }
                    co_return;
                }).launch(mThreadPool);
            } else {
                mDataMap[num] = info.data[0];
            }
            gmlib::papi::PlaceholderAPI::translate(mDataMap[num]);
        } catch (...) {}
    }
    if (config.title.updateInverval > 0) {
        ll::coro::keepThis([=]() -> ll::coro::CoroTask<> {
            while (true) {
                co_await std::chrono::seconds(config.title.updateInverval);
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
            []() -> ll::coro::CoroTask<> {
                sendSidebarToClients();
                co_return;
            }()
                        .syncLaunch(ll::thread::ServerThreadExecutor::getDefault());
        }
        co_return;
    }).launch(mThreadPool);
}

void disableMod() {
    mThreadPool.destroy();
    ll::service::getLevel()->forEachPlayer([&](Player& player) -> bool {
        auto pl = (gmlib::world::actor::GMPlayer*)&player;
        pl->removeClientSidebar();
        return true;
    });
}

void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance()
                    .getOrCreateCommand("sidebar", tr("sidebar.command.desc"), CommandPermissionLevel::Any);
    cmd.overload().execute([&](CommandOrigin const& origin, CommandOutput& output) {
        auto entity = (gmlib::world::actor::GMActor*)origin.getEntity();
        if (entity && entity->isPlayer()) {
            auto pl                             = (gmlib::world::actor::GMPlayer*)entity;
            auto res                            = !mPlayerSidebarStatus[pl->getUuid()];
            mPlayerSidebarStatus[pl->getUuid()] = res;
            if (!res) {
                pl->removeClientSidebar();
            }
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
    if (!setting) {
        auto player = (gmlib::world::actor::GMPlayer*)ll::service::getLevel()->getPlayer(uuid);
        if (player) {
            player->removeClientSidebar();
        }
    }
    saveSidebarStatus();
}

} // namespace GMSidebar
