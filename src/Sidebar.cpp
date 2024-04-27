#include "Entry.h"
#include "Global.h"

std::string                          mTitle;
std::unordered_map<int, std::string> mDataMap;
std::unordered_map<int, int>         mDataIndex;
ObjectiveSortOrder                   mOrder;
int                                  mTitleIndex = 0;

nlohmann::json mPlayerSidebarStatus;

using namespace ll::chrono_literals;

std::optional<ll::schedule::SystemTimeScheduler> mAsyncScheduler;
std::optional<ll::schedule::ServerTimeScheduler> mScheduler;

std::string tr(std::string key, std::vector<std::string> data, std::string translateKey) {
    return my_plugin::MyPlugin::getInstance()->translate(key, data, translateKey);
}

void saveSidebarStatus() {
    std::string path = "./plugins/GMSidebar/data/PlayerStatus.json";
    GMLIB::Files::JsonFile::writeFile(path, mPlayerSidebarStatus);
}

void loadSidebarStatus() {
    auto emptyFile       = nlohmann::json::object();
    mPlayerSidebarStatus = GMLIB::Files::JsonFile::initJson("./plugins/GMSidebar/data/PlayerStatus.json", emptyFile);
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
    // pl->setClientSidebar(title, dataList, mOrder);
    SetDisplayObjectivePacket("sidebar", "GMLIB_SIDEBAR_API", title, "dummy", mOrder).sendTo(*pl);

    std::vector<ScorePacketInfo> info;
    for (auto& key : dataList) {
        const ScoreboardId& id        = ScoreboardId(key.second);
        auto                text      = key.first;
        auto                scoreInfo = ScorePacketInfo();
        scoreInfo.mScoreboardId       = id;
        scoreInfo.mObjectiveName      = "GMLIB_SIDEBAR_API";
        scoreInfo.mIdentityType       = IdentityDefinition::Type::FakePlayer;
        scoreInfo.mScoreValue         = key.second;
        scoreInfo.mFakePlayerName     = text;
        info.emplace_back(scoreInfo);
    }
    auto pkt = SetScorePacket::change(info);
    pkt.sendTo(*pl);
}

void sendSidebarToClients() {
    GMLIB_Level::getInstance()->forEachPlayer([&](Player& player) -> bool {
        auto pl = (GMLIB_Player*)&player;
        pl->removeClientSidebar();
        if (!mPlayerSidebarStatus.contains(pl->getUuid().asString())) {
            mPlayerSidebarStatus[pl->getUuid().asString()] = true;
            saveSidebarStatus();
        }
        if (mPlayerSidebarStatus[pl->getUuid().asString()]) {
            sendSidebar(pl);
        }
        return true;
    });
}

void init() {
    mAsyncScheduler.emplace();
    mScheduler.emplace();
    auto& config = my_plugin::MyPlugin::getInstance()->getConfig();
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

void disablePlugin() {
    GMLIB_Level::getInstance()->forEachPlayer([&](Player& player) -> bool {
        auto pl = (GMLIB_Player*)&player;
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
        auto entity = (GMLIB_Actor*)origin.getEntity();
        if (entity && entity->isPlayer()) {
            auto pl                                        = (Player*)entity;
            auto res                                       = !mPlayerSidebarStatus[pl->getUuid().asString()];
            mPlayerSidebarStatus[pl->getUuid().asString()] = res;
            saveSidebarStatus();
            return output.success(res ? tr("sidebar.command.toggle.on") : tr("sidebar.command.toggle.off"));
        }
        return output.error(tr("sidebar.command.console"));
    }>();
}