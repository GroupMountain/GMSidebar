add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("groupmountain-repo https://github.com/GroupMountain/xmake-repo.git")

add_requires("levilamina 26.10.4", {configs = {target_type = get_config("target_type")}})
add_requires("levibuildscript")
add_requires("gmlib 26.10.0")
add_requires("cpp-httplib 0.15.3", {configs = {ssl = true, zlib = true}})

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("GMSidebar")
    add_rules("@levibuildscript/linkrule")
    add_rules("@levibuildscript/modpacker")
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204"
    )
    add_defines(
        "GMSidebar_EXPORTS",
        "NOMINMAX",
        "UNICODE"
    )
    add_packages(
        "levilamina",
        "levibuildscript",
        "gmlib",
        "cpp-httplib"
    )
    set_exceptions("none")
    set_kind("shared")
    set_languages("c++23")
    set_symbols("debug")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_includedirs("src")
