cmake_minimum_required(VERSION 3.10.0)

project(fractles C CXX)

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

add_executable(
    fractles
    Fractles.cpp

    drawing.h
    gradient.h
    gradienteditor.h
    gui.h
    image.h
    mycolors.h
    mymaths.h
    parameditor.h
    rendering.h
    textrenderer.h
    uicomponents.h
)

target_include_directories(
    fractles
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${SDL2_INCLUDE_DIRS}
    PUBLIC ${SDL2IAMGE_INCLUDE_DIRS}
)

target_link_libraries(fractles PUBLIC ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

