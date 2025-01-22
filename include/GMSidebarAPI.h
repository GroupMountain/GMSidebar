#pragma once
#include <mc/platform/UUID.h>
#include <iostream>

#ifdef GMSidebar_EXPORTS
#define GMSidebar_API __declspec(dllexport)
#else
#define GMSidebar_API __declspec(dllimport)
#endif

namespace GMSidebar {

[[nodiscard]] GMSidebar_API bool isPlayerSidebarEnabled(mce::UUID const& uuid);

[[maybe_unused]] GMSidebar_API void setPlayerSidebarEnabled(mce::UUID const& uuid, bool setting);

}