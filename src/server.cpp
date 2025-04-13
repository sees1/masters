#include "server.hpp"

ReturnCode::ReturnCode()
: cur_stat(ReturnCode::Status::Ok),
  cur_message("")
{ }

ReturnCode::ReturnCode(Status s)
: cur_stat(s),
  cur_message("")
{ }

ReturnCode::ReturnCode(Status s, std::string&& message)
: cur_stat(s),
  cur_message(std::move(message))
{ }

ReturnCode::ReturnCode(ReturnCode&& other)
: cur_stat(other.cur_stat),
  cur_message(std::move(other.cur_message))
{ }

ReturnCode::ReturnCode(const ReturnCode& other)
: cur_stat(other.cur_stat),
  cur_message(other.cur_message)
{ }

ReturnCode& ReturnCode::operator=(ReturnCode&& other)
{
  cur_message = std::move(other.cur_message);
  cur_stat = other.cur_stat;

  return *this;
}

ReturnCode::Status ReturnCode::status() const 
{
  return cur_stat;
}

std::string ReturnCode::str() const
{
  return cur_message;
}

Room::Room()
: cancel_talk(false)
{
  talk_td = std::thread(&Room::process, this);
}

Room::~Room()
{
  talk_td.join();
}

void Room::stop() {
  cancel_talk = true;
}

void Room::resume() {
  cancel_talk = false;
}

void Room::process() {
  while(cancel_talk == false)
  {
    std::unique_lock<std::mutex> guard(m);

    cv.wait(guard,
    [this] { 
      return std::any_of(participants_.begin(), participants_.end(),
                         [](auto&& p) {
                            if(p->isAlive())
                            {
                              std::lock_guard<std::mutex> part_lock(p->getWriteMutex());
                              return !p->getMessageQueue().empty();
                            }
                            
                            return false;
                         });
     });

    std::vector<ParticipantPtr> remove_list;

    std::for_each(participants_.begin(), participants_.end(),
    [this, &remove_list](auto&& participant)
    {
      if(participant->isAlive())
      {
        std::lock_guard<std::mutex> part_lock(participant->getWriteMutex());

        auto&& queue = participant->getMessageQueue();

        if (!queue.empty())
        {
          deliver(participant, queue);
          queue.clear();
        }
      }
      else
        remove_list.push_back(participant);
    });

    for(auto&& ex : remove_list)
      leave(ex);
  }
}

void Room::join(ParticipantPtr participant) {
  std::lock_guard<std::mutex> guard(m);

  participants_.insert(participant);

  participant->listen();

  for(auto&& message : message_history_)
    participant->write(message);
}

void Room::deliver(ParticipantPtr excluded, MessageQueue& messages_)
{
  message_history_.insert(message_history_.end(), messages_.begin(), messages_.end());

  while(message_history_.size() > MaxMessageQueue)
    message_history_.pop_front();

  auto&& excluded_iter = participants_.find(excluded);

  std::for_each(participants_.begin(), excluded_iter,
  [&messages_](auto&& participant)
  {
    for(auto&& message : messages_)
      participant->write(message);
  });

  if (excluded_iter != participants_.end())
  {
    std::advance(excluded_iter, 1);

    std::for_each(excluded_iter, participants_.end(),
    [&messages_](auto&& participant)
    {
      for(auto&& message : messages_)
        participant->write(message);
    });
  }
}

void Room::leave(ParticipantPtr ex) {
  participants_.erase(ex);
}

Participant::Participant(tcp::socket socket)
: alive_(true),
  socket_(std::move(socket))
{ }

MessageQueue& Participant::getMessageQueue()
{
  return messages_;
}

std::mutex& Participant::getWriteMutex()
{
  return write_m_;
}

bool Participant::isAlive() const
{
  return alive_;
}

void Participant::listen() {
  readHeader();
}

void Participant::readHeader() {
  boost::asio::async_read(socket_, boost::asio::buffer(last_message_.data(), Message::headerLength),
    [this](boost::system::error_code ec, size_t) {
      if(!ec && last_message_.decodeHeader())
        readBody();
      else
        alive_ = false;
    }
  );
}

void Participant::readBody() {
  boost::asio::async_read(socket_, boost::asio::buffer(last_message_.body(), last_message_.bodyLength()),
  [this](boost::system::error_code ec, size_t) {
    if(!ec)
    {
      std::lock_guard<std::mutex> write_lock(write_m_);
      messages_.push_back(last_message_);
      cv.notify_one();
      readHeader();
    }
    else
      alive_ = false;
  });
}

void Participant::write(const Message& message) {
  boost::asio::async_write(socket_, boost::asio::buffer(message.data(), message.length()),
  [this](boost::system::error_code ec, size_t) {
    if(ec)
      std::abort();
  });
}

Server::Server(boost::asio::io_context& io_context, const tcp::endpoint& endpoint)
: acceptor_(io_context, endpoint)
{
  do_accept();
}

void Server::do_accept() {
  acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
    if(!ec)
      room_.join(std::make_shared<Participant>(std::move(socket)));

    do_accept();
  });
}

int main(int argc, char* argv[]) {
  try {
    if(argc < 2) {
      cerr << "Usage: server <port> [<port> ...]\n";
      return 1;
    }

    boost::asio::io_context io_context;
    std::list<Server> servers;

    for(int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), atoi(argv[i]));
      servers.emplace_back(io_context, endpoint);
    }

    io_context.run();
  }
  catch (exception& e) {
    cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
