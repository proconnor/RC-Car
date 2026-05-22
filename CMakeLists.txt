cmake_minimum_required(VERSION 3.16)
project(EmbeddedCarSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZeroMQ REQUIRED libzmq)
find_package(Protobuf REQUIRED)

include_directories(
    ${Protobuf_INCLUDE_DIRS}
    ${ZeroMQ_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/proto
)
link_directories(${ZeroMQ_LIBRARY_DIRS})

add_subdirectory(car_sim)
add_subdirectory(controller_sim)
add_subdirectory(visualizer_sim)
