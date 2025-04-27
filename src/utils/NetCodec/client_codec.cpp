#include "utils/NetCodec/client_codec.hpp"

ClientCodec::ClientCodec(std::shared_ptr<boost::asio::io_context> context,
                         tcp::resolver::results_type&& endpoint,
                         std::shared_ptr<ThreadsafeDeque<Message>> ready_msg_storage)
: Codec(std::move(tcp::socket(*context)), ready_msg_storage),
  context_(context),
  endpoint_(std::make_unique<tcp::resolver::results_type>(std::move(endpoint)))
{
  connect();
}

void ClientCodec::encode(Message& msg)
{
  std::cout << "I'm in write" << std::endl;
  enc_message_ = enc_->encode(msg);

  boost::asio::async_write(*socket_, *(enc_message_.toVecOfConstBuf()),
    [this](boost::system::error_code ec, size_t)
    {
      if(ec)
      {
        is_alive_ = false;
        std::cout << ec.message() << std::endl;
        // socket_->close();
      }
    }
  );

  std::cout << "I'm after write" << std::endl;
}

void ClientCodec::connect()
{
  std::cout << "Im in connect" << std::endl;
  boost::asio::async_connect(*socket_, *endpoint_,
    [this](boost::system::error_code ec, tcp::endpoint)
    {
      if(!ec)
        decode();
      else
      {
        is_alive_ = false;
        std::cout << ec.message() << std::endl;
        // socket_->close();
      }
    }
  );

  std::cout << "Im after connect" << std::endl;
}

void ClientCodec::decode()
{
  decodeHeader();
}

void ClientCodec::decodeHeader()
{
  boost::asio::async_read(*socket_, boost::asio::buffer(dec_message_.getHead(), SerializedMessage::HeaderLength),
    [this](boost::system::error_code ec, size_t)
    {
      if(!ec)
      {
        std::cout << "In decodeHeader() func" << std::endl;
        dec_->decodeHeader(dec_message_);
        std::cout << "temp message size: " << dec_message_.getDataLength() << std::endl;
        decodeBody();
      }
      else
      {
        is_alive_ = false;
        std::cout << ec.message() << std::endl;
        // socket_->close();
      }
    }
  );
}

void ClientCodec::decodeBody()
{
  std::cout << "temp message size in decodeBody: " << dec_message_.getDataLength() << std::endl;

  boost::asio::async_read(*socket_, boost::asio::buffer(dec_message_.getData(), dec_message_.getDataLength() - 1),
    [this](boost::system::error_code ec, size_t)
    {
      if(!ec)
      {
        std::cout << "In decodeBody() func" << std::endl;
        Message decs = dec_->decodeData(dec_message_); 
        std::cout << decs.data << std::endl;
        std::cout << "In decodeBody() func end" << std::endl;
        ready_msgs_storage_->push_back(decs);
        decodeHeader();
      }
      else
      {
        std::cout << ec.message() << std::endl;
        is_alive_ = false;
        // socket_->close();
      }
    }
  );
}
