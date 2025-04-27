#include "utils/NetCodec/codec.hpp"

Codec::Codec(tcp::socket&& sock,
             std::shared_ptr<ThreadsafeDeque<Message>> storage)
: is_alive_(true),
  enc_(std::make_unique<Encoder>()),
  dec_(std::make_unique<Decoder>()),
  socket_(std::make_unique<tcp::socket>(std::move(sock))),
  ready_msgs_storage_(storage)
{
  std::cout << "Im in Codec ctr" << std::endl;
}

bool Codec::isAlive() const {
  return is_alive_;
}

void Codec::closeConnection() const {
  socket_->close();
}