add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("groupmountain-repo https://github.com/GroupMountain/xmake-repo.git")

add_requires("levilamina 1.6.0", {configs = {target_type = "server"}})
add_requires("levibuildscript 0.5.2")
add_requires("gmlib 1.6.0")

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
        "NOMINMAX",
        "UNICODE",
        "_HAS_CXX23=1",
        "GMSidebar_EXPORTS"
    )
    add_packages(
        "levilamina",
        "gmlib"
    )
    set_optimize("aggressive")
    set_exceptions("none")
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_includedirs("src")
    after_build(function (target)
        local origin = path.join(os.projectdir(), "lang")
        local target = path.join(os.projectdir(), "bin", target:name(), "lang")
        if os.exists(target) then os.rm(target) end
        os.cp(origin, target)
    end)