#include "keyboard.hpp"
#include "input_thread.hpp"
#include <thread>
#include <chrono>
#include <iostream>

// using clock = std::chrono::steady_clock;

uint64_t nowMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

void inputThread(InputState& input, std::atomic<bool>& running)
{
  Keyboard kb;
  char c;

  while(running.load())
  {
    // Reset every loop
    input.up = false;
    input.down = false;
    input.left = false;
    input.right = false;

    if(kb.readChar(c))
    {
      if(c == '\033')
      {
        char seq[2];
        if(kb.readChar(seq[0]) && kb.readChar(seq[1]))
        {
          if(seq[0] == '[')
          {
            uint64_t now = nowMs();
            switch (seq[1]) {
              case 'A':
                input.up = true;
                input.down = false;
                input.up_last_seen = now;
                break;
              case 'B':
                input.down = true;
                input.up = false;
                input.down_last_seen = now;
                break;
              case 'C':
                input.left = true;
                input.right = false;
                input.left_last_seen = now;
                break;
              case 'D':
                input.right = true;
                input.left = false;
                input.right_last_seen = now;
                break;
            }
          }
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}
