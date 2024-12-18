#include "Entry.h"
#include "GMSidebarAPI.h"
#include "Global.h"

std::string                          mTitle;
std::unordered_map<int, std::string> mDataMap;
std::unordered_map<int, int>         mDataIndex;
ObjectiveSortOrder                   mOrder;
int                                  mTitleIndex = 0;

std::unordered_map<mce::UUID, bool> mPlayerSidebarStatus;

using namespace ll::chrono_literals;

std::optional<ll::schedule::SystemTimeScheduler> mAsyncScheduler;
std::optional<ll::schedule::ServerTimeScheduler> mScheduler;

std::string tr(std::string key, std::vector<std::string> data, std::string translateKey) {
    return gmsidebar::Entry::getInstance()->translate(key, data, translateKey);
}

void saveSidebarStatus() {
    nlohmann::json json;
    for (auto& [key, val] : mPlayerSidebarStatus) {
        json[key.asString()] = val;
    }
    gmlib::utils::JsonUtils::writeFile("./mods/GMSidebar/data/PlayerStatus.json", json);
}

void loadSidebarStatus() {
    auto json =
        gmlib::utils::JsonUtils::initJson("./mods/GMSidebar/data/PlayerStatus.json", nlohmann::json::object());
    for (nlohmann::json::const_iterator it = json.begin(); it != json.end(); ++it) {
        if (it.value().is_boolean()) {
            auto uuid                  = mce::UUID::fromString(it.key());
            mPlayerSidebarStatus[uuid] = it.value().get<bool>();
        }
    }
}

void sendSidebar(gmlib::world::Player* pl) {
    std::vector<std::pair<std::string, int>> dataList;
    for (auto& [mapIndex, mapData] : mDataMap) {
        dataList.push_back({mapData, mapIndex});
    }
    for (auto& [data, index] : dataList) {
        gmlib::tools::PlaceholderAPI::translate(data, pl);
    }
    auto title = gmlib::tools::PlaceholderAPI::translateString(mTitle, pl);
    pl->setClientSidebar(title, dataList, mOrder);
}

void sendSidebarToClients() {
    gmlib::world::Level::getInstance()->forEachPlayer([&](Player& player) -> bool {
        auto pl = (gmlib::world::Player*)&player;
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
    mAsyncScheduler.emplace();
    mScheduler.emplace();
    auto& config = gmsidebar::Entry::getInstance()->getConfig();
    mOrder       = config.sortType;
    for (auto& [index, info] : config.sidebarInfo) {
        try {
            auto num = std::stoi(index);
            if (info.updateInverval > 0) {
                mAsyncScheduler->add<ll::schedule::RepeatTask>(
                    std::chrono::seconds::duration(info.updateInverval),
                    [=] {
                        mDataMap[num] = info.data[mDataIndex[num]];
                        mDataIndex[num]++;
                        if (mDataIndex[num] >= info.data.size()) mDataIndex[num] = 0;
                    }
                );
            } else {
                mDataMap[num] = info.data[0];
            }
            gmlib::tools::PlaceholderAPI::translate(mDataMap[num]);
        } catch (...) {}
    }
    if (config.title.updateInverval > 0) {
        mAsyncScheduler->add<ll::schedule::RepeatTask>(
            std::chrono::seconds::duration(config.title.updateInverval),
            [=] {
                mTitle = config.title.data[mTitleIndex];
                mTitleIndex++;
                if (mTitleIndex >= config.title.data.size()) mTitleIndex = 0;
            }
        );
    } else {
        mTitle = config.title.data[0];
    }
    mScheduler->add<ll::schedule::RepeatTask>(1s, [] { sendSidebarToClients(); });
}

void disableMod() {
    gmlib::world::Level::getInstance()->forEachPlayer([&](Player& player) -> bool {
        auto pl = (gmlib::world::Player*)&player;
        pl->removeClientSidebar();
        return true;
    });
    mAsyncScheduler.reset();
    mScheduler.reset();
}

void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance()
                    .getOrCreateCommand("sidebar", tr("sidebar.command.desc"), CommandPermissionLevel::Any);
    cmd.overload().execute<[&](CommandOrigin const& origin, CommandOutput& output) {
        auto entity = (gmlib::world::Actor*)origin.getEntity();
        if (entity && entity->isPlayer()) {
            auto pl                             = (Player*)entity;
            auto res                            = !mPlayerSidebarStatus[pl->getUuid()];
            mPlayerSidebarStatus[pl->getUuid()] = res;
            saveSidebarStatus();
            return output.success(res ? tr("sidebar.command.toggle.on") : tr("sidebar.command.toggle.off"));
        }
        return output.error(tr("sidebar.command.console"));
    }>();
}

namespace GMSidebar {

bool isPlayerSidebarEnabled(mce::UUID const& uuid) { return mPlayerSidebarStatus[uuid]; }

void setPlayerSidebarEnabled(mce::UUID const& uuid, bool setting) {
    mPlayerSidebarStatus[uuid] = setting;
    saveSidebarStatus();
}

}