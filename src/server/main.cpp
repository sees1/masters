#include "server/server.hpp"

int main(int argc, char* argv[]) {
  try {
    boost::asio::io_context io_context;
    
    // TODO: add yaml parse

    tcp::endpoint endpoint(tcp::v4(), 4000);
    
    RobotServerCreator creator(io_context, endpoint);

    io_context.run();
  }
  catch (exception& e) {
    cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}