cmake_minimum_required(VERSION 3.15)
project(nativestrapper CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")
file(GLOB_RECURSE LUA_SOURCES "external/lua/src/*.c")

list(REMOVE_ITEM LUA_SOURCES 
    "${CMAKE_SOURCE_DIR}/external/lua/src/lua.c"
    "${CMAKE_SOURCE_DIR}/external/lua/src/luac.c"
)

add_executable(nativestrapper ${SOURCES} ${HEADERS} ${LUA_SOURCES})

target_include_directories(nativestrapper PRIVATE include external external/lua/src)

target_link_libraries(nativestrapper PRIVATE Qt5::Widgets Qt5::Network)