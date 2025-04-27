#include <map>
#include <string>

#include "utils/message.hpp"

class Decoder {
public:
  // TODO: provide type_ and command_ map's here from yaml in main()
  Decoder();

  void decodeHeader(SerializedMessage& msg);
  Message decodeData(SerializedMessage& msg);

protected:
  std::map<uint8_t, std::string> inv_type_;
  std::map<uint8_t, std::string> inv_command_;
};