cmake_minimum_required(VERSION 3.5)

project(lsf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#指定msvc源码字符集
#https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
#You can use the /utf-8 option to specify both the source and execution character sets as encoded by using UTF-8.
#It's equivalent to specifying /source-charset:utf-8 /execution-charset:utf-8 on the command line.
#生成表达式
#https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html#manual:cmake-generator-expressions(7)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

#https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows
if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

set(BUILD_SHARED_LIBS on)
#CMAKE_SOURCE_DIR 源码顶级目录
#CMAKE_CURRENT_SOURCE_DIR   当前CMakeLists.txt文件对应的目录
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} src/mbuff.cpp
                            src/lexer.cpp
                            src/jsonparser.cpp
                            src/json.cpp
                            src/error.cpp
                            src/inner_imp.cpp
                            src/analyse.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE BUFFER_ARRAY_SIZE=256)
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MSVC_SPECIAL)
endif()
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(WITH_TEST on)
#set(WITH_TEST off)
message("enable test with : set(WITH_TEST on) ")

if(WITH_TEST)
#add test library doctest
add_subdirectory(doctest)

add_executable(mytest test/main.cpp #[[test/hello.cpp]] test/test_allocator.cpp)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
target_compile_options(mytest PRIVATE -pthread)
target_link_options(mytest PRIVATE -pthread)
endif()

target_link_libraries(mytest lsf)
target_link_libraries(mytest doctest_with_main)

configure_file(test/2.txt .)
endif()
