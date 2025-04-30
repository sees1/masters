#include "utils/message.hpp"

#include "docker_cpp/docker.h"

#include <nlohmann/json.hpp> // deserialization

#include <map>
#include <boost/process.hpp>
#include <optional>

class Executor;

using docker_cpp::Docker;
using docker_cpp::ASLHttp;

namespace bp = boost::process;

class RequestHandler {
public:
  RequestHandler();
  ~RequestHandler() = default;

  std::shared_ptr<Message> handleRequest(std::shared_ptr<Message> reciv_msg);

protected:
  std::shared_ptr<Executor> chooseExecutor(std::string& command);

protected:
  std::map<std::string, std::shared_ptr<Executor>> command_to_exec_;
  std::shared_ptr<Docker<ASLHttp>> docker_engine_ptr_;
};

// class ResponseHandler : public HandlerBase {
// public:
//   ResponseHandler();
//   ~ResponseHandler()

//   std::shared_ptr<Message> handleResponse(std::shared_ptr<Message> reciv_msg);

// private:
//   std::shared_ptr<
// }

class Executor {
public:
  explicit Executor(std::shared_ptr<Docker<ASLHttp>> docker_engine_copy)
  : docker_engine_copy_(docker_engine_copy)
  { }

  virtual ~Executor() = default;
  
  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) = 0;

protected:
  std::shared_ptr<Docker<ASLHttp>> docker_engine_copy_;
};

class RoboStartDDSExec : public Executor {
public:
  RoboStartDDSExec(std::shared_ptr<Docker<ASLHttp>> docker_engine)
  : Executor(docker_engine)
  { }

  virtual ~RoboStartDDSExec();

  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) override;

private:
  std::optional<bp::child> disc_serv_;
};

class RoboDeleteImageExec : public Executor {
public:
  RoboDeleteImageExec(std::shared_ptr<Docker<ASLHttp>> docker_engine)
  : Executor(docker_engine)
  { }

  virtual ~RoboDeleteImageExec() = default;

  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) override;
};

class RoboBuildImageExec : public Executor {
public:
  RoboBuildImageExec(std::shared_ptr<Docker<ASLHttp>> docker_engine)
  : Executor(docker_engine)
  { }

  virtual ~RoboBuildImageExec() = default;

  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) override;
};


class RoboContainerInfoExec : public Executor {
public:
  RoboContainerInfoExec(std::shared_ptr<Docker<ASLHttp>> docker_engine)
  : Executor(docker_engine)
  { }

  virtual ~RoboContainerInfoExec() = default;

  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) override;
};

class RoboContainerExec : public Executor {
public:
  RoboContainerExec(std::shared_ptr<Docker<ASLHttp>> docker_engine)
  : Executor(docker_engine)
  { }

  virtual ~RoboContainerExec() = default;

  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) override;
};

class RoboStopContainerExec : public Executor {
public:
  RoboStopContainerExec(std::shared_ptr<Docker<ASLHttp>> docker_engine)
  : Executor(docker_engine)
  { }

  virtual ~RoboStopContainerExec() = default;

  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) override;
};

class RoboStartedContainerInfoExec : public Executor {
public:
  RoboStartedContainerInfoExec(std::shared_ptr<Docker<ASLHttp>> docker_engine)
  : Executor(docker_engine)
  { }

  virtual ~RoboStartedContainerInfoExec() = default;

  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) override;
};


class RoboContainerLogExec : public Executor {
public:
  RoboContainerLogExec(std::shared_ptr<Docker<ASLHttp>> docker_engine)
  : Executor(docker_engine)
  { }

  virtual ~RoboContainerLogExec() = default;

  virtual std::shared_ptr<Message> execute(std::shared_ptr<Message> msg) override;
};