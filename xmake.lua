set_project("hcpp")
set_xmakever("2.8.8")

add_rules("mode.debug", "mode.release")

set_languages("c++20")
set_warnings("all")

option("enable_test")
option_end()

if has_config("enable_test") then
    add_requires("doctest >= 2.4.11")
end

if is_os("windows") then
    set_encodings("utf-8")
    add_defines("MSVC_SPECIAL")
elseif is_os("linux") then 
    -- set_allowedmodes("debug")
    -- set_defaultmode("debug")
    set_toolchains("clang")
end

target("lsf")
    set_kind("static")
    add_files("src/module_impl/*.cpp",{ public = true })
    add_files("src/public2/*.ixx",{ public = true })
    add_files("src/public/*.ixx",{ public = true })
    add_includedirs("src",{public = true})
    set_policy("build.c++.modules", true)
    add_ldflags("-static")

target("tt")
    set_default(false )
    add_files("test/main.cpp")
    add_tests("oo")
    add_tests("json",{packages = "doctest",
    remove_files="main.cpp",languages = "c++20",
    defines = "DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN",files="test/xmaketest.cpp"})
    -- add_files("test/xmaketest.cpp")
    -- add_packages("doctest")
    -- for _, testfile in ipairs(os.files("test/xmaketest.cpp")) do
    --     add_tests(path.basename(testfile), {
    --         kind = "binary",
    --         files = testfile,
    --         packages = "doctest",
    --         languages = "c++20",
    --         remove_files="main.cpp",
    --         defines = "DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN"})
    -- end
target("nmain")
    set_default(false )
    add_deps("lsf")
    add_packages("doctest")
    add_files("test/no_main.cpp",{defines = "DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN"})
    set_languages("c++20")
    add_tests("oox")
    set_policy("build.c++.modules", true)

