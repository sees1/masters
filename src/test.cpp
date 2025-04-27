#include "utils/codec.hpp"

int main()
{
  uint8_t i = 1;
  std::shared_ptr<Encoder> enc = std::make_shared<Encoder>(i);
  std::shared_ptr<Decoder> dec = std::make_shared<Decoder>();

  Message m;
  m.id = 0;
  m.type = "Request";
  m.command = "getRoboContainerInfo";
  m.data = "sdf";


  SerializedMessage ser_m = enc->encode(m);

  SerializedMessage ser_m_header_only(0);

  memcpy(ser_m_header_only.getHead(), ser_m.getHead(), SerializedMessage::HeaderLength);

  dec->decodeHeader(ser_m_header_only);


  memcpy(ser_m_header_only.getData(), ser_m.getData(), ser_m_header_only.getDataLength());

  Message dec_m = dec->decodeData(ser_m_header_only);

  
  std::cout << static_cast<int>(dec_m.id) << " " << dec_m.type << " " << dec_m.command << " " << dec_m.data << std::endl;
}