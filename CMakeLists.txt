cmake_minimum_required(VERSION 3.16)
project(EmbeddedCarSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZeroMQ REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/proto)

add_subdirectory(car_sim)
add_subdirectory(controller_sim)
add_subdirectory(visualizer_sim)
