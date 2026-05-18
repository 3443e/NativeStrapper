cmake_minimum_required(VERSION 3.15)
project(nativestrapper CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

add_executable(nativestrapper ${SOURCES} ${HEADERS})

target_include_directories(nativestrapper PRIVATE include)

target_link_libraries(nativestrapper PRIVATE Qt5::Widgets)