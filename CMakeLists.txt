cmake_minimum_required(VERSION 3.5)

project(lsf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#CMAKE_SOURCE_DIR 源码顶级目录
#CMAKE_CURRENT_SOURCE_DIR   当前CMakeLists.txt文件对应的目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#add_subdirectory(ccpp)
#message(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(${PROJECT_NAME} main.cpp
                                fp.cpp
                                kmp.cpp
                                test.cpp
                                fp.cpp
                                lexer.cpp)
target_link_libraries(${PROJECT_NAME} jsoncpp)
