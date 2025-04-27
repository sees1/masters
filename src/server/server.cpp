#include "server/server.hpp"

RobotServer::RobotServer(tcp::socket&& socket)
: ready_msgs_(std::make_shared<ThreadsafeDeque<Message>>()),
  codec_(std::make_unique<ServerCodec>(std::move(socket), ready_msgs_)),
  stop_td_(false)
{
  process_td_ = std::thread(&RobotServer::process, this);
}

void RobotServer::process()
{
  while(!stop_td_)
  {
    // isAlive();

    // TODO: add process Message
    std::shared_ptr<Message> request = ready_msgs_->wait_pop_back();

    std::shared_ptr<Message> resp = msg_handler_.handleRequest(request);

    codec_->encode(*resp);
  }
}

void RobotServer::isAlive()
{
  // TODO: ask docker_engine about status
  if (!codec_->isAlive())
  {
    stop_td_ = true;
    throw std::runtime_error("Our connection to client is die, terminate RobotServer!");
  }
}

RobotServerCreator::RobotServerCreator(boost::asio::io_context& io_context, const tcp::endpoint& endpoint)
: already_created_(false),
  acceptor_(io_context, endpoint)
{
  do_accept();
}

void RobotServerCreator::do_accept() {
  acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
    if(!ec && !already_created_)
    {
      srv = std::make_unique<RobotServer>(std::move(socket));
      already_created_ = true;
    }

    do_accept();
  });
}