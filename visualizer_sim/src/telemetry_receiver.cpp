#include "telemetry_receiver.hpp"
#include <iostream>

void telemetryReceiverThread(
    TelemetryState& state,
    std::mutex& state_mutex,
    std::atomic<bool>& running
) {
    zmq::context_t context(1);
    zmq::socket_t sub(context, zmq::socket_type::sub);

    sub.connect("tcp://localhost:5556");
    sub.set(zmq::sockopt::subscribe, "");
    std::cout << "Subscribe to car server" << std::endl;

    while (running.load()) {
        zmq::message_t msg;
        std::cout << "Waiting for message" << std::endl;
        if (!sub.recv(msg, zmq::recv_flags::none)) {
          std::cout << "not received" << std::endl;
            continue;
        }

        car::Telemetry telemetry;
        if (!telemetry.ParseFromArray(msg.data(), msg.size())) {
            std::cerr << "Failed to parse telemetry\n";
            continue;
        }

        std::lock_guard<std::mutex> lock(state_mutex);
        std::cout << "RECEIVED MESSAGE" << std::endl;
        // state.x = telemetry.x();
        // state.y = telemetry.y();
        state.heading = telemetry.measured_speed();
        state.speed = telemetry.battery_voltage();
    }
}
