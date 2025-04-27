#include <map>
#include <string>

#include "utils/message.hpp"

class Encoder {
public:
  // TODO: provide type_ and command_ map's here from yaml in main()
  Encoder();

  SerializedMessage encode(Message& msg);

protected:
  std::map<std::string, uint8_t> type_;
  std::map<std::string, uint8_t> command_;
};
