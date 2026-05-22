#pragma once

#include "input_state.hpp"
#include <atomic>

// Function declaration
void inputThread(InputState& input, std::atomic<bool>& running);
uint64_t nowMs();
