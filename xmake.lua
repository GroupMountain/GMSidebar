add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("groupmountain-repo https://github.com/GroupMountain/xmake-repo.git")

add_requires("levilamina 26.60.5", {configs = {target_type = get_config("target_type")}})
add_requires("levibuildscript")
add_requires("gmlib 26.60.0")
add_requires("cpp-httplib 0.53.1", {configs = {ssl = true, zlib = true}})

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("GMSidebar")
    add_rules("@levibuildscript/linkrule")
    if os.isfile(path.join(os.projectdir(), ".git", "HEAD")) then
        add_rules("@levibuildscript/modpacker")
    else
        add_rules("@levibuildscript/modpacker", {modVersion = "26.60.0"})
    end
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
    add_cxflags(
        "/EHs",--it will crash if it is not here
        "-Wno-microsoft-cast",
        "-Wno-invalid-offsetof",
        "-Wno-c++2b-extensions",
        "-Wno-microsoft-include",
        "-Wno-overloaded-virtual",
        "-Wno-ignored-qualifiers",
        "-Wno-missing-field-initializers",
        "-Wno-potentially-evaluated-expression",
        "-Wno-pragma-system-header-outside-header",
        {tools = {"clang_cl"}}
    )
    set_toolchains("clang-cl")

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

    after_build(function (target)
        local bindir  = path.join(os.projectdir(), "bin")
        local moddir  = path.join(bindir, "GMSidebar")
        local langdir = path.join(moddir, "lang")
        os.mkdir(moddir)
        if os.exists(langdir) then os.rm(langdir) end
        os.cp(path.join(os.projectdir(), "lang"), langdir)
        os.cp(path.join(os.projectdir(), "LICENSE"), bindir)
        os.cp(path.join(os.projectdir(), "README.md"), bindir)
    end)
