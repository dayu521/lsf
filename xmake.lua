set_project("hcpp")
set_xmakever("2.8.6")

add_rules("mode.debug", "mode.release")

set_languages("c++20")

add_requires("doctest >= 2.4.11")

if is_os("linux") then
    set_allowedmodes("debug")
    set_defaultmode("debug")
    set_toolchains("clang")
end 

target("lsf")
    set_kind("static")
    add_files("src/public2/*.ixx","src/module_impl/*.cpp","src/module_impl/*.cppm",{ install = true })
    add_files("src/public/*.ixx",{ install = true })
    add_includedirs("src/public",{public = true})
    set_policy("build.c++.modules", true)
    add_ldflags("-static")

    if is_os("windows") then
        add_defines("MSVC_SPECIAL")
        add_cxxflags("/source-charset:utf-8")
    end

target("tt")
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

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

