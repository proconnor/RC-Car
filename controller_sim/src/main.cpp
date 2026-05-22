// Controller_Sim
#include <zmq.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include "messages.pb.h"
#include <input_state.hpp>
#include <input_thread.hpp>

int main() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    zmq::context_t context(1);

    // Send commands
    zmq::socket_t push(context, zmq::socket_type::push);
    push.connect("tcp://localhost:5555");
    // push.connect("tcp://192.168.1.87:5555");

    // Receive telemetry
    // zmq::socket_t pull(context, zmq::socket_type::pull);
    zmq::socket_t pull(context, zmq::socket_type::sub);
    pull.connect("tcp://localhost:5556");
    // pull.connect("tcp://192.168.1.87:5556");
    pull.set(zmq::sockopt::subscribe, "");

    std::cout << "[CTRL] Connected. Sending commands..." << std::endl;

    float speed = 0.0f;
    float steer = 0.0f;

    constexpr float accel_rate = 5.0f;
    constexpr float steer_rate = 0.08f;
    constexpr float decay = 0.2f;

    // Pass Input and running to separate thread
    std::atomic<bool> running{true};
    InputState input;
    std::thread keyboardThread(inputThread, std::ref(input), std::ref(running));

    // Original sleep value
    // constexpr auto dt = std::chrono::milliseconds(16);
    constexpr auto dt = std::chrono::milliseconds(50);
    constexpr uint64_t HOLD_TIMEOUT_MS = 100;

    while(running.load())
    {
      auto start = std::chrono::steady_clock::now();

      uint64_t now = nowMs();

      input.up   = (now - input.up_last_seen.load()) < HOLD_TIMEOUT_MS;
      input.down = (now - input.down_last_seen.load()) < HOLD_TIMEOUT_MS;
      input.left = (now - input.left_last_seen.load()) < HOLD_TIMEOUT_MS;
      input.right = (now - input.right_last_seen.load()) < HOLD_TIMEOUT_MS;

      // Check value of input for speed and do math
      if(input.up)
      {
        speed += accel_rate;
        // std::cout << "Up Pressed. Speed is: " << speed << std::endl;
      }
      if(input.down)
      {
        speed -= accel_rate;
        // std::cout << "Down Pressed. Speed is: " << speed << std::endl;
      }
      // Check value for direction and do math
      if(input.left)
      {
        steer += steer_rate;
        // std::cout << "Left Pressed. Steer is: " << steer << std::endl;
      }
      if(input.right)
      {
        steer -= steer_rate;
        // std::cout << "Right Pressed. Steer is: " << steer << std::endl;
      }

      // Decay and return to center behavior only when no input
      if (!input.up && !input.down)
      {
          std::cout << "Return speed to 0" << std::endl;
          speed *= decay;
          std::cout << "Current speed is: " << speed << std::endl;
      }
      else
      {
        // std::cout << "Speed is being held in" << std::endl;
      }

      if(!input.left && !input.right)
      {
        // std::cout << "Return steer to 0" << std::endl;
        steer *= decay;
      }
      else
      {
        // std::cout << "Steer is being held in" << std::endl;
      }

      speed = std::clamp(speed, -1.0f, 1.0f);
      steer = std::clamp(steer, -1.0f, 1.0f);

      // std::cout << "Updated speed & steer: " << speed << " " << steer << std::endl;

      if(speed < 0.001 && speed > -0.001)
      {
        speed = 0;
      }

      if(steer < 0.001 && steer > -0.001)
      {
        steer = 0;
      }

      // Send updated speed and direction
      car::ControlCommand cmd;
      cmd.set_speed(speed);
      cmd.set_steer(steer);

      std::string serialized;
      cmd.SerializeToString(&serialized);
      push.send(zmq::buffer(serialized), zmq::send_flags::none);
      // Receive updated telemetry data (Future - make sure consistent with commands)
      zmq::message_t msg;
      if (pull.recv(msg, zmq::recv_flags::dontwait)) {
          car::Telemetry telem;
          if (telem.ParseFromArray(msg.data(), msg.size())) {
              std::cout << "[CTRL] Telemetry: speed=" << telem.measured_speed()
                        << " V=" << telem.battery_voltage() << std::endl;
          }
      }
      // std::this_thread::sleep_for(std::chrono::milliseconds(100));
      std::this_thread::sleep_until(start + dt);
    }

/*
    for (int i = 0; i < 100; ++i) {
        // Generate simple oscillating commands
        speed = std::sin(i * 0.1f);
        steer = std::cos(i * 0.1f) * 0.5f;

        car::ControlCommand cmd;
        cmd.set_speed(speed);
        cmd.set_steer(steer);

        std::string serialized;
        cmd.SerializeToString(&serialized);
        push.send(zmq::buffer(serialized), zmq::send_flags::none);

        zmq::message_t msg;
        if (pull.recv(msg, zmq::recv_flags::dontwait)) {
            car::Telemetry telem;
            if (telem.ParseFromArray(msg.data(), msg.size())) {
                std::cout << "[CTRL] Telemetry: speed=" << telem.measured_speed()
                          << " V=" << telem.battery_voltage() << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
*/

    std::cout << "[CTRL] Finished sending commands.\n";
    running = false;
    keyboardThread.join();
    return 0;
}
