#include <docker_cpp/docker_types.h> // deserialized docker types

#include <nlohmann/json.hpp> // deserialization

#include "utils/NetCodec/client_codec.hpp" // net codec
#include "client/mcontainer_setting.hpp"

using boost::asio::ip::tcp;

class RoboClient {
public:
  RoboClient(std::shared_ptr<boost::asio::io_context> context,
             tcp::resolver::results_type&& endpoint);
  ~RoboClient();

  std::pair<std::shared_ptr<std::map<std::string, std::string>>,
            std::shared_ptr<docker_cpp::ImageList>> getRoboContainersInfo();
  bool startContainer(std::string&& id, MContainerSetting settings);
  std::shared_ptr<docker_cpp::ContainerList> getExecutedContainers();
  bool createDDSInstance(int id);


private:
  std::shared_ptr<ThreadsafeDeque<Message>> ready_msgs_;

  std::unique_ptr<Codec> codec_;
};

class RoboClientCreator {
public:
  using executor_type = boost::asio::io_context::executor_type;
  using work_guard_type = boost::asio::executor_work_guard<executor_type>;
public:
  RoboClientCreator();

  std::shared_ptr<RoboClient> createRoboClient(std::string& ip, std::string& port);
  std::shared_ptr<boost::asio::io_context> getContext();
  std::shared_ptr<work_guard_type> getWorkGuard();

private:
  std::shared_ptr<boost::asio::io_context> context_;
  std::shared_ptr<work_guard_type> work_guard_;
  std::shared_ptr<tcp::resolver> resolver_;
};