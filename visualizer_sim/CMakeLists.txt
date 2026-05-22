cmake_minimum_required(VERSION 3.16)
project(visualizer_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(visualizer_sim src/main.cpp src/telemetry_receiver.cpp ${CMAKE_SOURCE_DIR}/proto/messages.pb.cc)

# Find SDL2 using sdl2-config
find_program(SDL2_CONFIG NAMES sdl2-config)
if(NOT SDL2_CONFIG)
    message(FATAL_ERROR "sdl2-config not found in PATH")
endif()

execute_process(COMMAND ${SDL2_CONFIG} --cflags
    OUTPUT_VARIABLE SDL2_CFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${SDL2_CONFIG} --libs
    OUTPUT_VARIABLE SDL2_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE)

target_compile_options(visualizer_sim PRIVATE ${SDL2_CFLAGS})
target_link_libraries(visualizer_sim PRIVATE ${SDL2_LIBS} ${ZeroMQ_LIBRARIES} ${Protobuf_LIBRARIES})
