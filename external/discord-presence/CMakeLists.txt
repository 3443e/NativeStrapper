cmake_minimum_required(VERSION 3.21)
project(discord-rpc)
set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} STATIC src/discord-rpc.cpp src/serialization.cpp src/command-queue.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)

# vendor fmt
add_subdirectory(third_party/fmt)

# vendor glaze (header only)
target_include_directories(${PROJECT_NAME} PRIVATE third_party/glaze/include)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE src/platform/windows.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)