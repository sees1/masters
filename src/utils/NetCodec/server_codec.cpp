#include "utils/NetCodec/server_codec.hpp"

ServerCodec::ServerCodec(tcp::socket&& socket,
                         std::shared_ptr<ThreadsafeDeque<Message>> storage)
: Codec(std::move(socket), storage)
{ 
  decode();
}

void ServerCodec::encode(Message& msg)
{
  enc_message_ = enc_->encode(msg);

  std::cout << msg.data << std::endl;

  boost::asio::async_write(*socket_, *enc_message_.toVecOfConstBuf(),
    [this](boost::system::error_code ec, size_t)
    {
      if(ec)
      {
        is_alive_ = false;
      }
      else
        std::cout << "encode error: " <<  ec.message() << std::endl;
    }
  );
}

void ServerCodec::decode()
{
  decodeHeader();
}

void ServerCodec::decodeHeader()
{
  boost::asio::async_read(*socket_, boost::asio::buffer(dec_message_.getHead(), SerializedMessage::HeaderLength),
    [this](boost::system::error_code ec, size_t)
    {
      if(!ec)
      {
        std::cout << "Decode header function" << std::endl;
        dec_->decodeHeader(dec_message_);
        decodeBody();
      }
      else
      {
        std::cout << "decode header error: " <<  ec.message() << std::endl;
        is_alive_ = false;
      }
    }
  );
}

void ServerCodec::decodeBody()
{
  boost::asio::async_read(*socket_, boost::asio::buffer(dec_message_.getData(), dec_message_.getDataLength() - 1),
    [this](boost::system::error_code ec, size_t)
    {
      if(!ec)
      {
        ready_msgs_storage_->push_back(dec_->decodeData(dec_message_));
        decodeHeader();
      }
      else
      {
        std::cout << "decode body error: " <<  ec.message() << std::endl;
        is_alive_ = false;
      }
    }
  );
}
