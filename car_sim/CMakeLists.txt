add_executable(car_sim
    src/main.cpp
    ${CMAKE_SOURCE_DIR}/proto/messages.pb.cc
)

target_link_libraries(car_sim
    PRIVATE ${ZeroMQ_LIBRARIES} ${Protobuf_LIBRARIES}
)
