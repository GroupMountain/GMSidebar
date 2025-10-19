#include "gmsidebar/Entry.h"
#include "gmsidebar/GMSidebar.h"
#include "ll/api/command/runtime/ParamKind.h"
#include "mc/server/commands/CommandOutput.h"
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/command/runtime/RuntimeCommand.h>
#include <ll/api/command/runtime/RuntimeOverload.h>
#include <ll/api/i18n/I18n.h>
#include <mc/world/actor/player/Player.h>


namespace gmsidebar {

void Entry::registerCmd() {
    using namespace ll::i18n_literals;
    auto& cmd = ll::command::CommandRegistrar::getInstance().getOrCreateCommand(
        "gmsidebar",
        "Turn on or off the sidebar"_tr()
    );
    cmd.alias("sidebar");
    cmd.runtimeOverload()
        .optional("status", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, auto&& self) {
            auto& instance = GMSidebar::getInstance();
            if (!instance.isEnabled()) {
                return output.success("The sidebar mod is not enabled."_trl(origin.getLocaleCode()));
            }
            auto* player = static_cast<Player*>(origin.getEntity());
            if (!player || !player->isPlayer()) {
                return output.error("You must be a player to execute this command."_trl(origin.getLocaleCode()));
            }
            auto status = self["status"].transform([](auto&& value) { return std::get<bool>(value); }
            ).value_or(!instance.isPlayerSidebarEnabled(player->getUuid()));
            instance.setPlayerSidebarEnabled(player->getUuid(), status);
            output.success(
                status ? "The sidebar is now enabled."_trl(origin.getLocaleCode())
                       : "The sidebar is now disabled."_trl(origin.getLocaleCode())
            );
        });
    cmd.runtimeOverload().text("cache").execute([](CommandOrigin const& origin, CommandOutput& output, auto&&) {
        auto& instance = GMSidebar::getInstance();
        if (!instance.isEnabled()) {
            return output.error("The sidebar mod is not enabled."_trl(origin.getLocaleCode()));
        }
        auto* player = static_cast<Player*>(origin.getEntity());
        if (!player || !player->isPlayer()) {
            return output.error("You must be a player to execute this command."_trl(origin.getLocaleCode()));
        }
        instance.clearPlayerCache(player->getUuid());
        output.success("The sidebar cache has been cleared."_trl(origin.getLocaleCode()));
    });
    cmd.runtimeOverload().text("disable").execute([](CommandOrigin const& origin, CommandOutput& output, auto&&) {
        if (origin.getPermissionsLevel() == CommandPermissionLevel::Any) {
            return output.error("You don't have permission to execute this command."_trl(origin.getLocaleCode()));
        }
        if (auto& instance = GMSidebar::getInstance(); instance.isEnabled()) {
            instance.disable();
            output.success("The sidebar mod is now disabled."_trl(origin.getLocaleCode()));
        } else {
            output.success("The sidebar mod is already disabled."_trl(origin.getLocaleCode()));
        }
    });
    cmd.runtimeOverload().text("enable").execute([](CommandOrigin const& origin, CommandOutput& output, auto&&) {
        if (origin.getPermissionsLevel() == CommandPermissionLevel::Any) {
            return output.error("You don't have permission to execute this command."_trl(origin.getLocaleCode()));
        }
        if (auto& instance = GMSidebar::getInstance(); !instance.isEnabled()) {
            instance.enable();
            output.success("The sidebar mod is now enabled."_trl(origin.getLocaleCode()));
        } else {
            output.success("The sidebar mod is already enabled."_trl(origin.getLocaleCode()));
        }
    });
    cmd.runtimeOverload().text("reload").execute([](CommandOrigin const& origin, CommandOutput& output, auto&&) {
        if (origin.getPermissionsLevel() == CommandPermissionLevel::Any) {
            return output.error("You don't have permission to execute this command."_trl(origin.getLocaleCode()));
        }
        if (auto& instance = GMSidebar::getInstance(); instance.isEnabled()) {
            instance.loadConfig(std::nullopt);
            instance.saveConfig(std::nullopt);
            output.success("The sidebar config has been reloaded."_trl(origin.getLocaleCode()));
        } else {
            output.error("The sidebar mod is not enabled."_trl(origin.getLocaleCode()));
        }
    });
    cmd.runtimeOverload()
        .text("as")
        .required("target", ll::command::ParamKind::Player)
        .optional("status", ll::command::ParamKind::Bool)
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            if (origin.getPermissionsLevel() == CommandPermissionLevel::Any) {
                return output.error("You don't have permission to execute this command."_trl(origin.getLocaleCode()
                ));
            }
            auto& instance = GMSidebar::getInstance();
            if (!instance.isEnabled()) {
                return output.error("The sidebar mod is not enabled."_trl(origin.getLocaleCode()));
            }
            auto targets = self["target"].get<ll::command::ParamKind::Player>().results(origin);
            if (targets.empty()) {
                return output.error("No target player found."_trl(origin.getLocaleCode()));
            }
            std::vector<std::string> on, off;
            for (auto* target : targets) {
                auto uuid   = target->getUuid();
                auto status = self["status"].transform([](auto&& value) { return std::get<bool>(value); }
                ).value_or(!instance.isPlayerSidebarEnabled(uuid));
                instance.setPlayerSidebarEnabled(uuid, status);
                (status ? on : off).push_back(target->getRealName());
            }
            if (!on.empty()) {
                output.success(
                    "The sidebar is now enabled for: {0}"_trl(origin.getLocaleCode(), fmt::join(on, ", "))
                );
            }
            if (!off.empty()) {
                output.success(
                    "The sidebar is now disabled for: {0}"_trl(origin.getLocaleCode(), fmt::join(off, ", "))
                );
            }
        });
    cmd.runtimeOverload()
        .text("as")
        .required("target", ll::command::ParamKind::Player)
        .text("status")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            if (origin.getPermissionsLevel() == CommandPermissionLevel::Any) {
                return output.error("You don't have permission to execute this command."_trl(origin.getLocaleCode()
                ));
            }
            auto& instance = GMSidebar::getInstance();
            if (!instance.isEnabled()) {
                return output.error("The sidebar mod is not enabled."_trl(origin.getLocaleCode()));
            }
            auto targets = self["target"].get<ll::command::ParamKind::Player>().results(origin);
            if (targets.empty()) {
                return output.error("No target player found."_trl(origin.getLocaleCode()));
            }
            std::vector<std::string> on, off;
            for (auto* target : targets) {
                (instance.isPlayerSidebarEnabled(target->getUuid()) ? on : off).push_back(target->getRealName());
            }
            if (!on.empty()) {
                output.success(
                    "The sidebar is currently enabled for: {0}"_trl(origin.getLocaleCode(), fmt::join(on, ", "))
                );
            }
            if (!off.empty()) {
                output.success(
                    "The sidebar is currently disabled for: {0}"_trl(origin.getLocaleCode(), fmt::join(off, ", "))
                );
            }
        });
    cmd.runtimeOverload()
        .text("as")
        .required("target", ll::command::ParamKind::Player)
        .text("cache")
        .execute([](CommandOrigin const& origin, CommandOutput& output, ll::command::RuntimeCommand const& self) {
            if (origin.getPermissionsLevel() == CommandPermissionLevel::Any) {
                return output.error("You don't have permission to execute this command."_trl(origin.getLocaleCode()
                ));
            }
            auto& instance = GMSidebar::getInstance();
            if (!instance.isEnabled()) {
                return output.error("The sidebar mod is not enabled."_trl(origin.getLocaleCode()));
            }
            auto targets = self["target"].get<ll::command::ParamKind::Player>().results(origin);
            if (targets.empty()) {
                return output.error("No target player found."_trl(origin.getLocaleCode()));
            }
            std::vector<std::string> cleared;
            for (auto* target : targets) {
                instance.clearPlayerCache(target->getUuid());
                cleared.push_back(target->getRealName());
            }
            output.success(
                "The sidebar cache has been cleared for: {0}"_trl(origin.getLocaleCode(), fmt::join(cleared, ", "))
            );
        });
}

} // namespace gmsidebar