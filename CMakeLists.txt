cmake_minimum_required(VERSION 3.10.0)

project(fractles C CXX)

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

add_executable(
    fractles
    src/Fractles.cpp
)

target_include_directories(
    fractles
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third-party/nlohmann-json
    PUBLIC ${SDL2_INCLUDE_DIRS}
    PUBLIC ${SDL2IMAGE_INCLUDE_DIRS}
)

target_link_libraries(fractles PUBLIC ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})

if(UNIX)
    target_compile_options(fractles PRIVATE -Wall -Wextra -Wpedantic)
endif()
