#pragma once

#include <atomic>
#include <mutex>
#include <zmq.hpp>
#include "messages.pb.h"

struct TelemetryState {
    float x = 0.f;
    float y = 0.f;
    float heading = 0.f;
    float speed = 0.f;
};

void telemetryReceiverThread(
    TelemetryState& state,
    std::mutex& state_mutex,
    std::atomic<bool>& running
);
