#pragma once

#include <vector>
#include <string>

struct MContainerSetting {
  std::string container_username_;
  std::string name_;
  std::string launch_dir_;
  std::string launch_;
  std::string arguments_;
  bool tty_;
};