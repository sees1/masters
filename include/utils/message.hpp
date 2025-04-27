#ifndef message_HPP
#define message_HPP

#include <cstdlib>
#include <memory>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

#include <boost/asio.hpp>

using namespace std;

struct Message {
  std::string type;
  std::string command;
  std::string data;
};

class SerializedMessage {
public:
  enum { HeaderLength = 6 };

public:
  SerializedMessage();
  SerializedMessage(uint32_t body_length);
  SerializedMessage(const SerializedMessage& other);
  SerializedMessage(SerializedMessage&& other);

  SerializedMessage& operator=(const SerializedMessage& other);

  ~SerializedMessage();

  bool empty() const;

  const char* getData() const;
  char* getData();
  const char* getHead() const;
  char* getHead();
  uint32_t getDataLength() const;

  void setDataLength(uint32_t body_length);

  std::shared_ptr<std::vector<boost::asio::const_buffer>> toVecOfConstBuf() const;

private:
  char header_[HeaderLength];
  char* data_;
  uint32_t body_length_;
};

#endif
