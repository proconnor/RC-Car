// Car Sim
#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include "messages.pb.h"
#include <cmath>      // For std::abs, std::sin, std::cos, std::tan
#include <algorithm>  // For std::clamp and std::max

struct CarState
{
  float x = 0.0f;          // meters
  float y = 0.0f;          // meters
  float heading = 0.0f;    // radians (0 = facing +X)

  // Motion
  float speed = 0.0f;          // m/s
  float steering_angle = 0.0f; // radians

  // Power
  float battery_voltage = 12.6f;
};

struct VehicleParams {
    float wheel_base = 0.30f;          // meters
    float max_speed = 3.0f;             // m/s
    float max_accel = 2.0f;              // m/s²
    float max_decel = 3.0f;              // m/s²
    float max_steering = 0.6f;           // radians (~34°)
    float steering_rate = 2.5f;          // rad/s
    float drag = 0.2f;                   // simple linear drag
};

void updatePhysics(CarState& state, float speed, float steer, const VehicleParams& params, float dt)
{
    // --- Steering dynamics ---
    float target_steer = steer * params.max_steering;
    float steer_error = target_steer - state.steering_angle;

    float max_steer_delta = params.steering_rate * dt;
    steer_error = std::clamp(steer_error, -max_steer_delta, max_steer_delta);
    state.steering_angle += steer_error;

    // --- Longitudinal dynamics ---
    /* float accel = 0.0f;
    if (speed > 0.0f) {
        accel = speed * params.max_accel;
    } else {
        accel = speed * params.max_decel;
    }

    // Simple drag
    accel -= params.drag * state.speed;

    state.speed += accel * dt;
    */

    // --- Improved Longitudinal dynamics ---
    float accel = 0.0f;
        const float BRAKE_MULTIPLIER = 4.0f; // 4x stronger than normal acceleration

        if (std::abs(speed) > 0.01f) {
            // NORMAL DRIVE: Apply acceleration based on command
            accel = speed * params.max_accel;
        }
        else {
            // ACTIVE BRAKING: No command, but still moving
            if (std::abs(state.speed) > 0.05f) {
                // Apply a heavy counter-force against the current direction of travel
                float brake_force = params.max_accel * BRAKE_MULTIPLIER;
                accel = (state.speed > 0) ? -brake_force : brake_force;
            } else {
                // SNAP TO ZERO: Stop the "jitter" when almost stopped
                state.speed = 0;
                accel = 0;
            }
        }

        // Apply forces
        state.speed += accel * dt;

        // Ensure braking doesn't accidentally make the car start reversing
        if (speed == 0 && ((accel < 0 && state.speed < 0) || (accel > 0 && state.speed > 0))) {
            state.speed = 0;
        }

        state.speed = std::clamp(state.speed, -params.max_speed, params.max_speed);

    // --- Yaw rate (bicycle model) ---
    float yaw_rate = 0.0f;
    if (std::fabs(state.steering_angle) > 1e-3f) {
        yaw_rate = (state.speed / params.wheel_base) *
                   std::tan(state.steering_angle);
    }

    // --- Integrate pose ---
    state.heading += yaw_rate * dt;

    // Keep heading bounded
    if (state.heading > M_PI)  state.heading -= 2.0f * M_PI;
    if (state.heading < -M_PI) state.heading += 2.0f * M_PI;

    state.x += state.speed * std::cos(state.heading) * dt;
    state.y += state.speed * std::sin(state.heading) * dt;

    // --- Battery drain (simple model) ---
    state.battery_voltage -= std::fabs(accel) * dt * 0.01f;
    state.battery_voltage = std::max(state.battery_voltage, 9.0f);
}

int main() {

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    zmq::context_t context(1);

    // Listen for commands
    zmq::socket_t sub(context, zmq::socket_type::pull);
    sub.bind("tcp://*:5555");

    // Publish telemetry
    zmq::socket_t pub(context, zmq::socket_type::pub);
    pub.bind("tcp://*:5556");

    std::cout << "[CAR] Listening for control commands..." << std::endl;

    float measured_speed = 0.0f;

    CarState car_state;
    VehicleParams vehicleParams;

    while (true) {
        zmq::message_t msg;
        if (sub.recv(msg, zmq::recv_flags::none)) {
            car::ControlCommand cmd;
            if (cmd.ParseFromArray(msg.data(), msg.size())) {
                // Simulate applying command
                std::cout << "[CAR] Received command for speed\n\n" << std::endl;
                measured_speed += (cmd.speed() - measured_speed) * 0.3f;
                float battery_voltage = 12.3f;

                if(measured_speed < 0.001 && measured_speed > -0.001)
                {
                  measured_speed = 0;
                }


                updatePhysics(car_state, cmd.speed(), cmd.steer(), vehicleParams, 0.01);

                // Send telemetry
                car::Telemetry telemetry;
                telemetry.set_x(car_state.x);
                telemetry.set_y(car_state.y);
                telemetry.set_heading(car_state.heading);
                telemetry.set_measured_speed(car_state.speed);
                telemetry.set_steering_angle(car_state.steering_angle);
                telemetry.set_battery_voltage(car_state.battery_voltage);
                // telemetry.set_timestamp_ns(nowNs());

                std::string serialized;
                telemetry.SerializeToString(&serialized);

                pub.send(zmq::buffer(serialized), zmq::send_flags::none);

                std::cout << "[CAR] Speed cmd=" << cmd.speed()
                          << " measured=" << measured_speed
                          << " steer=" << cmd.steer() << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
