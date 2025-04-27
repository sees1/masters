#include "utils/NetCodec/server_codec.hpp"
#include "server/message_handler.hpp"

using boost::asio::ip::tcp;

class RobotServer {
public:
  RobotServer(tcp::socket&& socket);
  ~RobotServer() = default;

private:
  void isAlive();
  void accept_connect();
  void process();

private:
  std::shared_ptr<ThreadsafeDeque<Message>> ready_msgs_;
  std::unique_ptr<Codec> codec_;

  RequestHandler msg_handler_;

  std::atomic_bool stop_td_;
  std::thread process_td_;
};

class RobotServerCreator {
public:
  RobotServerCreator(boost::asio::io_context& io_context, const tcp::endpoint& endpoint);
  
private:
  void do_accept();
  
private:
  bool already_created_;

  std::unique_ptr<RobotServer> srv;
  tcp::acceptor acceptor_;
};