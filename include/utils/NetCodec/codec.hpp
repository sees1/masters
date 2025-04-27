#include <iostream>
#include <cstdio>
#include <map>
#include <string>
#include <optional>

#include "utils/NetCodec/decoder.hpp"
#include "utils/NetCodec/encoder.hpp"
#include "utils/threadsafe_deque.hpp"

using boost::asio::ip::tcp;

class Codec {
public:
  Codec(tcp::socket&& sock,
        std::shared_ptr<ThreadsafeDeque<Message>> storage);

  bool isAlive() const;
  void closeConnection() const;

  virtual void encode(Message& msg) = 0;

protected:
  virtual void decode() = 0;

protected:
  bool is_alive_;

  SerializedMessage enc_message_;
  SerializedMessage dec_message_;

  std::unique_ptr<Encoder> enc_;
  std::unique_ptr<Decoder> dec_;

  std::unique_ptr<tcp::socket> socket_;
  std::shared_ptr<ThreadsafeDeque<Message>> ready_msgs_storage_;
};