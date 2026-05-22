#pragma once
#include <atomic>

struct InputState
{
  std::atomic<bool> up{false};
  std::atomic<bool> down{false};
  std::atomic<bool> left{false};
  std::atomic<bool> right{false};

  std::atomic<uint64_t> up_last_seen{0};
  std::atomic<uint64_t> down_last_seen{0};
  std::atomic<uint64_t> left_last_seen{0};
  std::atomic<uint64_t> right_last_seen{0};
};
