#include "utils/message.hpp"

SerializedMessage::SerializedMessage()
: data_(nullptr),
  body_length_(0) 
{ }

SerializedMessage::SerializedMessage(uint32_t body_length)
{
  body_length_ = body_length;
  data_ = new char[body_length];
}

SerializedMessage::SerializedMessage(const SerializedMessage& other)
{
  memcpy(header_, other.header_, HeaderLength);

  body_length_ = other.body_length_;

  data_ = new char[body_length_];

  memcpy(data_, other.data_, body_length_);
}

SerializedMessage::SerializedMessage(SerializedMessage&& other)
{
  memcpy(header_, other.header_, HeaderLength);

  std::swap(data_, other.data_);

  body_length_ = other.body_length_;
}

SerializedMessage& SerializedMessage::operator=(const SerializedMessage& other)
{
  memcpy(header_, other.header_, HeaderLength);

  body_length_ = other.body_length_;

  if(data_ != nullptr)
    delete[] data_;

  data_ = new char[body_length_];

  memcpy(data_, other.data_, body_length_);

  return *this;
}

SerializedMessage::~SerializedMessage()
{
  if (data_ != nullptr)
    delete[] data_;
}

bool SerializedMessage::empty() const {
  return (body_length_ == 0);
}

const char* SerializedMessage::getData() const {
  return data_;
}

char* SerializedMessage::getData() {
  return data_;
}

const char* SerializedMessage::getHead() const {
  return header_;
}

char* SerializedMessage::getHead() {
  return header_;
}

void SerializedMessage::setDataLength(uint32_t body_length) {
  if(data_ != nullptr)
  {
    body_length_ = body_length;

    delete[] data_;
    
    data_ = new char[body_length_];
  }
  else
  {
    body_length_ = body_length;

    data_ = new char[body_length_];
  }
}

uint32_t SerializedMessage::getDataLength() const {
  return body_length_;
}

std::shared_ptr<std::vector<boost::asio::const_buffer>> 
SerializedMessage::toVecOfConstBuf() const
{
  std::shared_ptr<std::vector<boost::asio::const_buffer>> res;
  res = std::make_shared<std::vector<boost::asio::const_buffer>>();

  res->push_back(boost::asio::buffer(header_));
  res->push_back(boost::asio::buffer(data_, body_length_));

  return res;
}