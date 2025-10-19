#pragma once
#include <ll/api/Expected.h>
#include <ll/api/base/CompilerPredefine.h>
#include <ll/api/base/Containers.h>
#include <ll/api/mod/NativeMod.h>
#include <mc/platform/UUID.h>
#include <mc/world/scores/ObjectiveSortOrder.h>
#include <optional>
#ifndef GMSidebar_EXPORTS
#  include <ll/api/mod/ModManagerRegistry.h>
#  include <ll/api/mod/NativeMod.h>
#endif

namespace gmsidebar {

class GMSidebar {
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
    GMSidebar();
    ~GMSidebar();

public:
    struct Config {
    public:
        struct Info {
            std::vector<std::string> content         = {};
            ullong                   update_interval = 800;
        };

    public:
        int                version          = 2;
        bool               default_enabled  = true;
        ObjectiveSortOrder sort_type        = ObjectiveSortOrder::Ascending;
        ullong             refresh_interval = 800;
        // clang-format off
        Info               title     = {{
            "§1§g|§5欢迎您加入! §g|",
            "§1§g/§5欢迎您加入! §g/",
            "§1§g-§5欢迎您加入! §g-",
            "§1§g\\§5欢迎您加入! §g\\"
        }};
        ll::SmallDenseMap<std::string, Info> objectives = {{
            {"0", {{"§e#§d#§b#§g#个§e人§d信§b息§e#§d#§b#§g#"}, 0}},
            {"1", {{"§e你好,§g${papi:player_realname}"}, 0}},
            {"2", {{"§g: §3${papi:player_llmoney}"}}},
            {"3", {{"§e#§d#§b#§g#其§e它§d信§b息§e#§d#§b#§g#"}, 0}},
            {"4", {{
                "§g平  台: §5${papi:player_device}",
                "§g延  迟: ${papi:dict,input=${papi:compare,operator=>,left=${papi:player_avg_ping},right=1000},true=§4,false=§2}${papi:player_avg_ping}ms",
                "§g版  本: §6${papi:server_version}"
            }}},
            {"5", {{
                "§g掉落物: §4${papi:dict,input=${papi:compare,operator=>,left=${papi:server_total_entities,type=minecraft:item},right=1000},true=§4,false=§2}${papi:server_total_entities,type=minecraft:item},§g生  物:§4${papi:server_total_entities}",
                "§g玩  家: §5${papi:server_online}"
            }, 3000}},
            {"6", {{
                "§gTps : §5${papi:server_tps}§g ♪     ",
                "§gTps : §5${papi:server_tps}§g    ♪  ",
                "§gTps : §5${papi:server_tps}§g      ♪",
                "§gTps : §5${papi:server_tps}§g    ♪  ",
                "§gTps : §5${papi:server_tps}§g ♪     "
            }}},
            {"7", {{
                "§3${papi:server_time,format=%d}§g号,§g时间:§3${papi:server_time}"
            }}}}
        };
        // clang-format on
    };

public:
#ifdef GMSidebar_EXPORTS
    LL_SHARED_EXPORT static GMSidebar& getInstance() noexcept;
#else
    inline static GMSidebar& getInstance() {
        static auto& instance = []() -> GMSidebar& {
            // clang-format off
            auto mod = ll::mod::ModManagerRegistry::getInstance().getMod("GMSidebar");
            if (!mod) throw std::runtime_error("GMSidebar mod not found.");
            if (mod->getType() != ll::mod::NativeModManagerName) throw std::runtime_error("GMSidebar mod is not a native mod.");
            auto handle = static_cast<ll::mod::NativeMod&>(*mod).getHandle();
            auto func = reinterpret_cast<ll::sys_utils::DynamicLibrary*>(&handle)->getAddress<GMSidebar& (*)()>("getInstance");
            if (!func) throw std::runtime_error("GMSidebar getInstance function not found.");
            return func();
            // clang-format on
        }();
        return instance;
    }
#endif

    virtual bool isEnabled();
    virtual void enable();
    virtual void disable();

    // 中文： 以下接口在插件未启用下不可调用
    // English: These interfaces cannot be called when the plugin is not enabled.
    virtual Config& getConfig();
    virtual void    loadConfig(std::optional<std::filesystem::path> const& path);
    virtual void    saveConfig(std::optional<std::filesystem::path> const& path);
    virtual void    loadData(std::optional<std::filesystem::path> const& path);
    virtual void    saveData(std::optional<std::filesystem::path> const& path);
    virtual bool    isPlayerSidebarEnabled(mce::UUID const& uuid);
    virtual void    setPlayerSidebarEnabled(mce::UUID const& uuid, bool enable);
    virtual void    clearPlayerCache(mce::UUID const& uuid);
    virtual void    clearAllPlayerCache();
};

} // namespace gmsidebar