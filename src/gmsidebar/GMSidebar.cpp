#include "gmsidebar/GMSidebar.h"
#include "gmsidebar/Entry.h"
#include <gmlib/gm/papi/PlaceholderAPI.h>
#include <gmlib/gm/utils/StringUtils.h>
#include <gmlib/mc/network/BinaryStream.h>
#include <ll/api/Config.h>
#include <ll/api/coro/CoroTask.h>
#include <ll/api/coro/InterruptableSleep.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerDisconnectEvent.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/thread/ServerThreadExecutor.h>
#include <ll/api/utils/ErrorUtils.h>
#include <ll/api/utils/HashUtils.h>
#include <mc/deps/ecs/gamerefs_entity/GameRefsEntity.h>
#include <mc/entity/components/ActorOwnerComponent.h>
#include <mc/nbt/CompoundTagVariant.h>
#include <mc/network/packet/ScorePacketType.h>
#include <mc/server/ServerPlayer.h>
#include <mc/world/level/Level.h>
#include <mc/world/scores/IdentityDefinition.h>
#include <ranges>

namespace gmsidebar {

struct GMSidebar::Impl {
public:
    struct PlayerCache {
        std::pair<size_t, std::string>                                 mTitle;
        ll::SmallDenseMap<std::string, std::pair<size_t, std::string>> mContent;
    };

public:
    std::string                               mObjectiveName{"GMSidebar"};
    Config                                    mConfig;
    ll::SmallDenseMap<mce::UUID, bool>        mPlayerSidebarEnabled;
    std::atomic_bool                          mRunning{true};
    ll::coro::InterruptableSleep              mSleep;
    ll::SmallDenseMap<mce::UUID, PlayerCache> mPlayerCache;
    ll::event::ListenerPtr                    mListener;
    ll::thread::ServerThreadExecutor          mExecutor{
        Entry::getInstance().getSelf().getName(),
        std::chrono::milliseconds{30},
        16
    };

public:
    Impl() {
        {
            auto path = Entry::getInstance().getSelf().getConfigDir() / u8"config.json";
            loadConfig(path);
            saveConfig(path);
        }
        {
            auto path = Entry::getInstance().getSelf().getDataDir() / u8"data.nbt";
            loadData(path);
            saveData(path);
        }

        ll::coro::keepThis([&]() -> ll::coro::CoroTask<> {
            while (mRunning.load()) {
                co_await mSleep.sleepFor(std::chrono::milliseconds{mConfig.refresh_interval});
                if (!mRunning.load()) break;

                ll::service::getLevel()->forEachPlayer([&](Player& player) -> bool {
                    if (player.isSimulated()) return mRunning.load();
                    if (auto it = mPlayerSidebarEnabled.find(player.getUuid());
                        (it != mPlayerSidebarEnabled.end() && !it->second) || !mConfig.default_enabled) {
                        return mRunning.load();
                    }
                    updatePlayerSidebar(player);
                    return mRunning.load();
                });
            }
            co_return;
        }).launch(mExecutor);

        mListener = ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerDisconnectEvent>(
            [&](ll::event::PlayerDisconnectEvent& event) -> void {
                mPlayerSidebarEnabled.erase(event.self().getUuid());
            }
        );
    }
    ~Impl() {
        mRunning.store(false);
        mSleep.interrupt();
        ll::event::EventBus::getInstance().removeListener(mListener);
        gmlib::GMBinaryStream removeObjectiveStream;
        removeObjectiveStream.writePacketHeader(MinecraftPacketIds::RemoveObjective);
        removeObjectiveStream.writeString(mObjectiveName);
        ll::service::getLevel()->forEachPlayer([&](Player& player) -> bool {
            if (player.isSimulated()) return mRunning.load();
            if (auto it = mPlayerSidebarEnabled.find(player.getUuid());
                (it != mPlayerSidebarEnabled.end() && !it->second) || !mConfig.default_enabled) {
                return true;
            }
            removeObjectiveStream.sendTo(player);
            return true;
        });
    }
    void loadConfig(std::filesystem::path const& path) {
        auto& logger = Entry::getInstance().getSelf().getLogger();
        try {
            ll::config::loadConfig(mConfig, path, [](Config& config, nlohmann::ordered_json& json) -> bool {
                static constexpr auto rename = [](auto&& json, auto&& oldName, auto&& newName) -> void {
                    if (!json.contains(oldName)) return;
                    json[newName] = std::move(json[oldName]);
                    json.erase(oldName);
                };
                static constexpr auto renameInfo = [](auto&& json) -> void {
                    if (json.contains("data")) rename(json, "data", "content");
                    if (json.contains("updateInverval") && json["updateInverval"].is_number()) {
                        json["update_interval"] = json["updateInverval"].template get<llong>() * 1000;
                    }
                };
                if (json.contains("version") && json["version"].get<int>() == 1) {
                    rename(json, "sortType", "sort_type");
                    if (json.contains("title")) renameInfo(json["title"]);
                    if (json.contains("sidebarInfo") && json["sidebarInfo"].is_object()) {
                        for (auto& [key, value] : json["sidebarInfo"].items()) {
                            renameInfo(value);
                        }
                        rename(json, "sidebarInfo", "objectives");
                    }
                }
                config.objectives.clear();
                config.title.content.clear();
                return ll::config::defaultConfigUpdater(config, json);
            });
            for (auto& key : mConfig.objectives | std::views::filter([](auto&& pair) {
                                 return !gmlib::string_utils::isInteger(pair.first);
                             }) | std::views::keys) {
                mConfig.objectives.erase(key);
            }
        } catch (...) {
            logger.error("Failed to load config file: {0}", ll::string_utils::u8str2str(path.u8string()));
            ll::error_utils::printCurrentException(logger);
        }
    }
    void saveConfig(std::filesystem::path const& path) { ll::config::saveConfig(mConfig, path); }
    void loadData(std::filesystem::path const& path) {
        auto uuids = mPlayerSidebarEnabled | std::views::filter([](auto&& pair) { return pair.second; })
                   | std::views::keys | std::ranges::to<ll::SmallDenseSet<mce::UUID>>();
        auto& logger = Entry::getInstance().getSelf().getLogger();
        try {
            if (auto content = ll::file_utils::readFile(path, true); content) {
                if (auto nbt = CompoundTag::fromBinaryNbt(*content); nbt) {
                    if (auto result = ll::reflection::deserialize(
                            mPlayerSidebarEnabled,
                            CompoundTagVariant{std::move(*nbt)}
                        );
                        !result) {
                        logger.error("Failed to deserialize data file");
                        result.error().log(logger);
                    }
                } else {
                    logger.error("Failed to load data file");
                    nbt.error().log(logger);
                }
            } else {
                mPlayerSidebarEnabled.clear();
            }
        } catch (...) {
            logger.error("Failed to load data file");
            ll::error_utils::printCurrentException(logger);
        }

        if (uuids.empty()) return;
        if (auto level = ll::service::getLevel(); level) {
            gmlib::GMBinaryStream removeObjectiveStream;
            removeObjectiveStream.writePacketHeader(MinecraftPacketIds::RemoveObjective);
            removeObjectiveStream.writeString(mObjectiveName);
            level->forEachPlayer([&](Player& player) -> bool {
                if (uuids.contains(player.getUuid())
                    && (!mPlayerSidebarEnabled.contains(player.getUuid())
                        || !mPlayerSidebarEnabled[player.getUuid()])) {
                    removeObjectiveStream.sendTo(player);
                    mPlayerCache.erase(player.getUuid());
                }
                return true;
            });
        }
    }
    void saveData(std::filesystem::path const& path) {
        ll::file_utils::writeFile(
            path,
            ll::reflection::serialize<CompoundTagVariant>(mPlayerSidebarEnabled)->get<CompoundTag>().toBinaryNbt(),
            true
        );
    }
    void updatePlayerSidebar(Player& player) {
        // clang-format off
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
        auto& cache = mPlayerCache[player.getUuid()];
        // clang-format on

        if (cache.mContent.empty() || cache.mContent.size() != mConfig.objectives.size()) {
            cache.mContent.clear();
            cache.mContent.reserve(mConfig.objectives.size());
            for (auto& [key, info] : mConfig.objectives) {
                cache.mContent.try_emplace(key);
            }
        }

        bool sendAll = false;

        auto updateContent = [&](auto& config, auto& cache) -> bool {
            if (config.content.empty()) return false;
            if (auto interval = config.update_interval; !interval) {
                if (cache.second.empty()) {
                    // clang-format off
                    cache.second = std::move(gmlib::PlaceholderAPI::translate(
                        std::string{config.content[0]},
                        player,
                        {},
                        {
                            {"currentIndex", "0"},
                            {"rawContent", config.content[0]},
                            {"updateInterval", "0"},
                            {"contentSize", std::to_string(config.content.size())}
                        }
                    ));
                    // clang-format on
                    return true;
                }
            } else {
                auto length   = config.content.size();
                auto newIndex = length > 1 ? (now / interval) % length : 0;
                ll::DenseMap<
                    std::string,
                    std::variant<
                        std::string,
                        std::function<std::optional<std::string>(
                            optional_ref<Actor>               actor,
                            ll::StringMap<std::string> const& params,
                            std::string const&                language
                        )>>>
                    params = {
                        {"currentIndex",   std::to_string(newIndex)             },
                        {"rawContent",     config.content[newIndex]             },
                        {"updateInterval", std::to_string(interval)             },
                        {"contentSize",    std::to_string(config.content.size())}
                };
                if (newIndex != cache.first) {
                    if (auto newValue = gmlib::PlaceholderAPI::translate(
                            std::string{config.content[newIndex]},
                            player,
                            {},
                            params
                        );
                        newValue != cache.second) {
                        cache.second = std::move(newValue);
                        cache.first  = newIndex;
                        return true;
                    }
                    cache.first = newIndex;
                } else if (length == 1) {
                    if (auto newValue =
                            gmlib::PlaceholderAPI::translate(std::string{config.content[0]}, player, {}, params);
                        newValue != cache.second) {
                        cache.second = std::move(newValue);
                        return true;
                    }
                }
            }
            return false;
        };

        // 处理标题更新
        if (updateContent(mConfig.title, cache.mTitle)) {
            gmlib::GMBinaryStream removeObjectiveStream;
            removeObjectiveStream.writePacketHeader(MinecraftPacketIds::RemoveObjective);
            removeObjectiveStream.writeString(mObjectiveName);
            removeObjectiveStream.sendTo(player);

            gmlib::GMBinaryStream addObjectiveStream;
            addObjectiveStream.writePacketHeader(MinecraftPacketIds::SetDisplayObjective);
            addObjectiveStream.writeString("sidebar");
            addObjectiveStream.writeString(mObjectiveName);
            addObjectiveStream.writeString(cache.mTitle.second);
            addObjectiveStream.writeString("dummy");
            addObjectiveStream.writeVarInt(mConfig.sort_type);
            addObjectiveStream.sendTo(player);

            sendAll = true;
        }

        // 处理内容更新
        std::vector<std::pair<std::string,size_t>> updated;
        for (auto& [index, info] : mConfig.objectives) {
            if (auto it = cache.mContent.find(index); it != cache.mContent.end()) {
                if (updateContent(info, it->second) || sendAll) {
                    updated.emplace_back(
                        index,
                        ll::hash_utils::HashCombiner{}.addRange(mObjectiveName).addRange(index).hash()
                    );
                }
            }
        }
        if (!updated.empty()) {
            if (!sendAll) {
                gmlib::GMBinaryStream removeScoreStream;
                removeScoreStream.writePacketHeader(MinecraftPacketIds::SetScore);
                removeScoreStream.writeUnsignedChar(ScorePacketType::Remove);
                removeScoreStream.writeUnsignedVarInt(updated.size());
                for (auto& index : updated) {
                    removeScoreStream.writeVarInt64(index.second);
                    removeScoreStream.writeString(mObjectiveName);
                    removeScoreStream.writeUnsignedInt(0);
                }
                removeScoreStream.sendTo(player);
            }

            gmlib::GMBinaryStream addScoreStream;
            addScoreStream.writePacketHeader(MinecraftPacketIds::SetScore);
            addScoreStream.writeUnsignedChar(ScorePacketType::Change);
            addScoreStream.writeUnsignedVarInt(updated.size());
            for (auto& index : updated) {
                addScoreStream.writeVarInt64(index.second);
                addScoreStream.writeString(mObjectiveName);
                addScoreStream.writeUnsignedInt(std::stoi(index.first));
                addScoreStream.writeUnsignedChar(IdentityDefinition::Type::FakePlayer);
                addScoreStream.writeString(cache.mContent[index.first].second);
            }
            addScoreStream.sendTo(player);
        }
    }
};

GMSidebar::GMSidebar() : pImpl(nullptr) {}
GMSidebar::~GMSidebar() = default;

GMSidebar& GMSidebar::getInstance() noexcept {
    static GMSidebar instance;
    return instance;
}

bool GMSidebar::isEnabled() { return pImpl != nullptr; }

void GMSidebar::enable() {
    if (!pImpl) pImpl = std::make_unique<Impl>();
}
void GMSidebar::disable() {
    if (pImpl) pImpl.reset();
}

GMSidebar::Config& GMSidebar::getConfig() {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    return pImpl->mConfig;
}
void GMSidebar::loadConfig(std::optional<std::filesystem::path> const& path) {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    pImpl->loadConfig(path.value_or(Entry::getInstance().getSelf().getConfigDir() / u8"config.json"));
    pImpl->mPlayerCache.clear();
}
void GMSidebar::saveConfig(std::optional<std::filesystem::path> const& path) {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    pImpl->saveConfig(path.value_or(Entry::getInstance().getSelf().getConfigDir() / u8"config.json"));
}
void GMSidebar::loadData(std::optional<std::filesystem::path> const& path) {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    pImpl->loadData(path.value_or(Entry::getInstance().getSelf().getDataDir() / u8"data.nbt"));
}
void GMSidebar::saveData(std::optional<std::filesystem::path> const& path) {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    pImpl->saveData(path.value_or(Entry::getInstance().getSelf().getDataDir() / u8"data.nbt"));
}
bool GMSidebar::isPlayerSidebarEnabled(mce::UUID const& uuid) {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    if (auto it = pImpl->mPlayerSidebarEnabled.find(uuid); it != pImpl->mPlayerSidebarEnabled.end()) {
        return it->second;
    }
    return pImpl->mConfig.default_enabled;
}
void GMSidebar::setPlayerSidebarEnabled(mce::UUID const& uuid, bool enable) {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    pImpl->mPlayerSidebarEnabled[uuid] = enable;
    saveData(std::nullopt);
    if (enable) return;
    pImpl->mPlayerCache.erase(uuid);
    if (auto level = ll::service::getLevel(); level) {
        if (auto* player = level->getPlayer(uuid); player) {
            gmlib::GMBinaryStream removeObjectiveStream;
            removeObjectiveStream.writePacketHeader(MinecraftPacketIds::RemoveObjective);
            removeObjectiveStream.writeString(pImpl->mObjectiveName);
            removeObjectiveStream.sendTo(*player);
        }
    }
    saveData(std::nullopt);
}
void GMSidebar::clearPlayerCache(mce::UUID const& uuid) {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    pImpl->mPlayerCache.erase(uuid);
}

void GMSidebar::clearAllPlayerCache() {
    if (!pImpl) throw std::runtime_error("GMSidebar is not enabled");
    pImpl->mPlayerCache.clear();
}

} // namespace gmsidebar

extern "C" LL_SHARED_EXPORT gmsidebar::GMSidebar* getInstance() { return &gmsidebar::GMSidebar::getInstance(); }