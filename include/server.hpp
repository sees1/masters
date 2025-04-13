#include <iostream>
#include <cstdlib>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <set>
#include <utility>
#include <boost/asio.hpp>
#include "message.hpp"

class Participant;

using boost::asio::ip::tcp;

using MessageQueue = std::deque<Message>;

std::condition_variable cv;

class ReturnCode {
public:
  enum Status {Ok = 0, Warn, Error};

public:
  ReturnCode();
  ReturnCode(Status s);
  ReturnCode(Status s, std::string&& message);
  ReturnCode(ReturnCode&& other);
  ReturnCode(const ReturnCode& other);

  ReturnCode& operator=(ReturnCode&& other);

  Status status() const;
  std::string str() const;

private:
  Status cur_stat;
  std::string cur_message;
};

class Room {
public:
  using ParticipantPtr = std::shared_ptr<Participant>;
  enum { MaxMessageQueue = 200 };

public:
  Room();
  ~Room();

  void stop();
  void resume();

  void process();
  void join(ParticipantPtr participant);
  void leave(ParticipantPtr ex);
  void deliver(ParticipantPtr excluded, MessageQueue& messages_);

private:
  MessageQueue message_history_;
  std::set<ParticipantPtr> participants_;
  std::thread talk_td;
  std::atomic_bool cancel_talk;
  std::mutex m;
};


class Participant {
public:
  Participant(tcp::socket socket);

  bool isAlive() const;

  void listen();
  MessageQueue& getMessageQueue();
  std::mutex& getWriteMutex();
  void write(const Message& message);

private:
  void readHeader();
  void readBody();

private:
  bool alive_;
  tcp::socket socket_;
  Message last_message_;
  MessageQueue messages_;

  std::mutex write_m_;
};


class Server {
public:
  Server(boost::asio::io_context& io_context, const tcp::endpoint& endpoint);
  
private:
  void do_accept();
  
private:
  tcp::acceptor acceptor_;
  Room room_;
  std::atomic_bool stop_read_;
};
