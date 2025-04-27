#include "utils/NetCodec/codec.hpp"

class ClientCodec : public Codec {
public:
  ClientCodec(std::shared_ptr<boost::asio::io_context> context,
              tcp::resolver::results_type&& endpoint,
              std::shared_ptr<ThreadsafeDeque<Message>> ready_msg_storage);

  void encode(Message& msg) override;

protected:
  void connect();
  void decode() override;

private:
  void decodeHeader();
  void decodeBody();

protected:
  std::shared_ptr<boost::asio::io_context> context_;
  std::unique_ptr<tcp::resolver::results_type> endpoint_;
};