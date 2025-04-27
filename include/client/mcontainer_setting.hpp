#pragma once

#include <vector>
#include <string>

struct MContainerSetting {
  std::string name_;
  std::vector<std::string> command_;
  bool tty_;
};