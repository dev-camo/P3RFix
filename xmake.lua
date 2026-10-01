local name = "P3RFix"

set_project(name)
add_rules("mode.debug", "mode.release")
set_languages("cxxlatest", "clatest")
set_optimize("smallest")

option("fix_version")
    set_default("1.4.0")
    set_showmenu(true)
    set_description("Version embedded in P3RFix (set from the release tag in CI)")
option_end()

target("zydis")
    set_kind("static")
    before_build(function (target)
        local arch = is_arch("x64") and "x64" or "Win32"
        local builddir = "build/zydis/" .. arch
        os.execv("cmake", {"-S", "external/zydis", "-B", builddir,
            "-G", "Visual Studio 17 2022", "-A", arch,
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded",
            "-DZYDIS_BUILD_SHARED_LIB=OFF", "-DZYDIS_BUILD_EXAMPLES=OFF",
            "-DZYDIS_BUILD_TOOLS=OFF", "-DZYDIS_BUILD_TESTS=OFF",
            "-DZYDIS_BUILD_DOXYGEN=OFF"})
        os.execv("cmake", {"--build", builddir, "--config", "Release"})
    end)
    on_load(function (target)
        target:add("includedirs", "external/zydis/include", "external/zydis/dependencies/zycore/include", {public = true})
        local arch = is_arch("x64") and "x64" or "Win32"
        local builddir = "build/zydis/" .. arch
        target:add("links", builddir .. "/Release/Zydis.lib", builddir .. "/zycore/Release/Zycore.lib", {public = true})
        target:add("defines", "ZYDIS_STATIC_BUILD", "ZYCORE_STATIC_BUILD", {public = true})
    end)

target(name)
    set_kind("shared")
    set_prefixname("")
    set_extension(".asi")
    add_files("src/*.cpp", "external/safetyhook/src/*.cpp",
        "src/unreal/Integration.cpp", "src/unreal/detail/Runtime.cpp",
        "src/render/Scaling.cpp", "src/input/RawMousePacket.cpp")
    add_syslinks("user32")
    add_deps("zydis") 
    on_load(function (target)
        local version = get_config("fix_version")
        assert(version and version:match("^%d+%.%d+%.%d+$"), "fix_version must have the form X.Y.Z")
        target:add("defines", 'P3RFIX_VERSION="' .. version .. '"')
    end)
    
    add_includedirs("external/spdlog/include", "external/inipp", "external/safetyhook/include")

    if is_plat("windows") then
        set_toolchains("msvc")
        set_runtimes("MT")
        add_cxflags("/utf-8", "/GL")
        add_ldflags("/LTCG", "/OPT:REF", "/OPT:ICF")
    end

-- Synthetic memory tests have no game, hooking, logging, or Zydis dependency.
target("unreal-integration-tests")
    set_kind("binary")
    set_default(false)
    add_files("tests/unreal_integration_tests.cpp", "src/unreal/Integration.cpp", "src/unreal/detail/Runtime.cpp")
    add_includedirs("src")
    if is_plat("windows") then
        set_toolchains("msvc")
        set_runtimes("MT")
        add_cxflags("/utf-8", "/GL")
        add_ldflags("/LTCG", "/OPT:REF", "/OPT:ICF")
    end

-- Exercise the shipped scaling and Windows packet-reader boundaries without the game.
target("fix-behavior-tests")
    set_kind("binary")
    set_default(false)
    add_files("tests/fix_behavior_tests.cpp", "src/render/Scaling.cpp",
        "src/input/RawMousePacket.cpp")
    add_includedirs("src")
    add_syslinks("user32")
    if is_plat("windows") then
        set_toolchains("msvc")
        set_runtimes("MT")
        add_cxflags("/utf-8", "/W4", "/WX", "/GL")
        add_ldflags("/LTCG", "/OPT:REF", "/OPT:ICF")
    end
