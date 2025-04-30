#include "utils/NetCodec/encoder.hpp"
#include <iostream>

Encoder::Encoder()
{
  type_.insert(std::make_pair("Hello", 1));
  type_.insert(std::make_pair("RespHello", 2));
  type_.insert(std::make_pair("Response", 3));
  type_.insert(std::make_pair("Request", 4));

  command_.insert(std::make_pair("No command", 0));
  command_.insert(std::make_pair("getRoboContainerInfo", 1));
  command_.insert(std::make_pair("execRoboContainer", 2));
  command_.insert(std::make_pair("getExecutedRoboContainer", 3));
  command_.insert(std::make_pair("execDDSServer", 4));
  command_.insert(std::make_pair("deleteRoboImage", 5));
  command_.insert(std::make_pair("buildRoboImage", 6));
  command_.insert(std::make_pair("stopRoboContainer", 7));
  command_.insert(std::make_pair("getRoboLog", 8));
}

SerializedMessage Encoder::encode(Message& msg)
{
  if (type_.find(msg.type) == type_.end())
    throw std::runtime_error("Can't encode message without propriate type!");

  std::optional<uint8_t> command;

  if (command_.find(msg.command) == command_.end())
  {
    if (type_[msg.type] == 4)
      throw std::runtime_error("Can't encode request without command!");
  }
  else
  {
    if(type_[msg.type] < 3)
      throw std::runtime_error("Can't encode this type of message with command!");
    else
      command.emplace(command_[msg.command]);
  }
    
  uint32_t data_size = msg.data.size();
  std::cout << "Enc data size in Encoder: " << static_cast<int>(data_size) << std::endl;

  SerializedMessage m(data_size);

  char type_c[1];
  type_c[0] = static_cast<char>(type_[msg.type]);

  char command_c[1];
  if (command.has_value())
    command_c[0] = static_cast<char>(command_[msg.command]);
  else
    command_c[0] = static_cast<char>(command_["No command"]);

  char* size_b = new char[4];

  size_b[0] = (data_size >> 24) & 0xFF;
  size_b[1] = (data_size >> 16) & 0xFF;
  size_b[2] = (data_size >> 8) & 0xFF;
  size_b[3] = (data_size & 0xFF);

  memcpy(m.getHead(), type_c, 1);
  memcpy(m.getHead() + 1, command_c, 1);
  memcpy(m.getHead() + 2, size_b, 4);
  memcpy(m.getData(), msg.data.c_str(), data_size);

  return m;
}