#pragma once

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

class Keyboard {
public:
  Keyboard()
  {
    tcgetattr(STDIN_FILENO, &orig_);
    termios raw = orig_;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  }

  ~Keyboard()
  {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_);
  }

  bool readChar(char &c)
  {
    return read(STDIN_FILENO, &c, 1) == 1;
  }

private:
  termios orig_;

};
