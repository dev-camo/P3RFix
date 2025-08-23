local name = "P3RFix"

set_project(name)
add_rules("mode.debug", "mode.release")
set_languages("cxxlatest", "clatest")
set_optimize("smallest")

target("zydis")
    set_kind("static")
    before_build(function (target)
        if not os.isfile("external/zydis/build/CMakeCache.txt") then
            local arch = is_arch("x64") and "x64" or "Win32"
            os.exec("cmake -S external/zydis -B external/zydis/build -A " .. arch .. " -DZYDIS_BUILD_SHARED_LIB=OFF")
        end
        os.exec("cmake --build external/zydis/build --config Release")
    end)
    on_load(function (target)
        target:add("includedirs", "external/zydis/include", "external/zydis/dependencies/zycore/include", {public = true})
        target:add("links", "external/zydis/build/Release/Zydis.lib", "external/zydis/build/zycore/Release/Zycore.lib", {public = true})
        target:add("defines", "ZYDIS_STATIC_BUILD", "ZYCORE_STATIC_BUILD", {public = true})
    end)

target(name)
    set_kind("shared")
    set_prefixname("")
    set_extension(".asi")
    add_files("src/*.cpp", "external/safetyhook/src/*.cpp", "src/SDK/Engine_functions.cpp", "src/SDK/CoreUObject_functions.cpp", "src/SDK/Basic.cpp")
    add_syslinks("user32")
    add_deps("zydis") 
    
    add_includedirs("external/spdlog/include", "external/inipp", "external/safetyhook/include")

    if is_plat("windows") then
        set_toolchains("msvc")
        set_runtimes("MT")
        add_cxflags("/utf-8", "/GL")
        add_ldflags("/LTCG", "/OPT:REF", "/OPT:ICF")
    end