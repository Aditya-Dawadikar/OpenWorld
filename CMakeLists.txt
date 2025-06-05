cmake_minimum_required(VERSION 3.10)
project(openworld)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)

# SDL2
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

include_directories(
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

link_directories(
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_IMAGE_LIBRARY_DIRS}
)

file(GLOB SOURCES src/*.cpp)

add_executable(openworld ${SOURCES})

target_link_libraries(openworld
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)
