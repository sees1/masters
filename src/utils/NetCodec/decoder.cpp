#include "utils/NetCodec/decoder.hpp"

Decoder::Decoder()
{
  inv_type_.insert(std::make_pair(1, "Hello"));
  inv_type_.insert(std::make_pair(2, "RespHello"));
  inv_type_.insert(std::make_pair(3, "Response"));
  inv_type_.insert(std::make_pair(4, "Request"));

  inv_command_.insert(std::make_pair(0, "No command"));
  inv_command_.insert(std::make_pair(1, "getRoboContainerInfo"));
  inv_command_.insert(std::make_pair(2, "execRoboContainer"));
  inv_command_.insert(std::make_pair(3, "getExecutedRoboContainer"));
  inv_command_.insert(std::make_pair(4, "execDDSServer"));
}

void Decoder::decodeHeader(SerializedMessage& msg)
{
  char data_size_c[4];
  memcpy(data_size_c, msg.getHead() + 2, 4);

  uint32_t data_size = static_cast<uint32_t>(static_cast<uint8_t>(data_size_c[0]) << 24 |
                                             static_cast<uint8_t>(data_size_c[1]) << 16 | 
                                             static_cast<uint8_t>(data_size_c[2]) << 8  |
                                             static_cast<uint8_t>(data_size_c[3]));

  msg.setDataLength(data_size + 1);
}

Message Decoder::decodeData(SerializedMessage& msg)
{
  char type_c[1];
  char command_c[1];

  memcpy(type_c, msg.getHead(), 1);
  memcpy(command_c, msg.getHead() + 1, 1);

  Message m;

  m.type = inv_type_[static_cast<uint8_t>(type_c[0])];
  m.command = inv_command_[static_cast<uint8_t>(command_c[0])];
  msg.getData()[msg.getDataLength() - 1] = '\0';
  m.data = std::string(msg.getData());

  return m;
}