#include "utils/NetCodec/codec.hpp"

class ServerCodec : public Codec {
public:
  ServerCodec(tcp::socket&& socket,
              std::shared_ptr<ThreadsafeDeque<Message>> storage);

  void encode(Message& msg) override;

protected:
  void decode() override;

private:
  void decodeHeader();
  void decodeBody();
};