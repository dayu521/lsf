cmake_minimum_required(VERSION 3.28)

project(lsf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
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

#打开作为共享库编译
#set(BUILD_SHARED_LIBS on)
#CMAKE_SOURCE_DIR 源码顶级目录
#CMAKE_CURRENT_SOURCE_DIR   当前CMakeLists.txt文件对应的目录
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# file(GLOB src  src/old/*.cpp)

# set(target_name ${PROJECT_NAME}-old)
# add_library(${target_name} ${src})

# target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/old)

# target_compile_definitions(${target_name} PRIVATE BUFFER_ARRAY_SIZE=256)
# if(MSVC)
#     target_compile_definitions(${target_name} PRIVATE MSVC_SPECIAL)
# endif()

# https://www.kitware.com/import-cmake-the-experiment-is-over/
#clang17以下需要,不包含17
# if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
#     message("clang detected")
#     set(CMAKE_CXX_EXTENSIONS OFF)
# endif()

file(GLOB module_ixx  "src/public/*.ixx" "src/public2/*.ixx" )
file(GLOB module_cpp 
    "src/module_impl/analyze.cpp"
    "src/module_impl/json_src.cpp"
    "src/module_impl/jsonparser.cpp"
    "src/module_impl/tree_allocator.cpp")

set(target_name2 ${PROJECT_NAME})
add_library(${target_name2} "src/module_impl/json_i.cpp" "src/module_impl/json_m.cpp")

target_include_directories(${target_name2} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_sources(${target_name2} PUBLIC
FILE_SET CXX_MODULES
FILES  ${module_ixx} ${module_cpp})

target_compile_definitions(${target_name2} PRIVATE BUFFER_ARRAY_SIZE=256)

if(MSVC)
    target_compile_definitions(${target_name2} PRIVATE MSVC_SPECIAL)
endif()

find_package(doctest REQUIRED)

file(GLOB tests_src  "test/tt2.cpp" )  #测试单元

add_executable(doc_test ${tests_src})

target_link_libraries(doc_test PRIVATE doctest::doctest)
target_link_libraries(doc_test PRIVATE ${target_name2})
