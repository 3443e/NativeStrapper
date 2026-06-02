cmake_minimum_required(VERSION 3.15)
project(nativestrapper CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
endif()

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(CURL REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "include/*.hpp")
file(GLOB_RECURSE LUA_SOURCES "external/lua/src/*.c")

set(DISCORD_RPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/discord-presence)

list(REMOVE_ITEM LUA_SOURCES
    "${CMAKE_SOURCE_DIR}/external/lua/src/lua.c"
    "${CMAKE_SOURCE_DIR}/external/lua/src/luac.c"
)

list(FILTER SOURCES EXCLUDE REGEX "/URIHandlers/platform_default_handler_.*\\.cpp$")

set(EXTRA_SOURCES
    external/lfs/lfs.c
    external/miniz/miniz.c
)

set(PLATFORM_HANDLER_SOURCES "")
set(MACOSX_BUNDLE_FLAG "")

if(APPLE)
    set(MACOSX_BUNDLE_FLAG MACOSX_BUNDLE)
    set(PLATFORM_HANDLER_SOURCES
        src/URIHandlers/platform_default_handler.h
        src/URIHandlers/platform_default_handler_mac.mm
    )
elseif(WIN32)
    list(APPEND EXTRA_SOURCES
        "${CMAKE_SOURCE_DIR}/assets/NativeStrapper.rc"
    )
    # set(PLATFORM_HANDLER_SOURCES src/URIHandlers/platform_default_handler_win.cpp)
elseif(UNIX)
    # set(PLATFORM_HANDLER_SOURCES src/URIHandlers/platform_default_handler_linux.cpp)
endif()

add_executable(nativestrapper ${MACOSX_BUNDLE_FLAG}
    ${SOURCES}
    ${HEADERS}
    ${LUA_SOURCES}
    ${EXTRA_SOURCES}
    ${PLATFORM_HANDLER_SOURCES}
)

if(APPLE)
    set_target_properties(nativestrapper PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    )

    set_source_files_properties(
        src/URIHandlers/platform_default_handler_mac.mm
        PROPERTIES COMPILE_FLAGS "-fobjc-arc"
    )

    target_link_libraries(nativestrapper PRIVATE
        "-framework AppKit"
        "-framework CoreServices"
    )
endif()

target_include_directories(nativestrapper PRIVATE
    include
    external
    external/lua/src
    external/lfs
    external/miniz
    external/discord-presence/include
    src/URIHandlers
)

target_link_libraries(nativestrapper PRIVATE
    Qt5::Widgets
    discord-rpc
    CURL::libcurl
)