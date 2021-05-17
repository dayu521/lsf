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

#CMAKE_SOURCE_DIR 源码顶级目录
#CMAKE_CURRENT_SOURCE_DIR   当前CMakeLists.txt文件对应的目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#add_subdirectory(ccpp)
#message(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(${PROJECT_NAME} main.cpp
                                mbuff.cpp
                                test.cpp
                                lexer.cpp
                                jsonparser.cpp
                                json.cpp
                                error.cpp
                                inner_imp.cpp
                                analyse.cpp)
#target_link_libraries(${PROJECT_NAME} jsoncpp)
