add_executable(controller_sim
    src/main.cpp
    src/input_thread.cpp
    ${CMAKE_SOURCE_DIR}/proto/messages.pb.cc
)

target_include_directories(controller_sim
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(controller_sim
    PRIVATE zmq ${Protobuf_LIBRARIES}
)
