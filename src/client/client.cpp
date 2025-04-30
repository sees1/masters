#include "client/client.hpp"

using nlohmann::json_abi_v3_11_3::json;

RoboClient::RoboClient(std::shared_ptr<boost::asio::io_context> context,
                       tcp::resolver::results_type&& endpoint)
: ready_msgs_(std::make_shared<ThreadsafeDeque<Message>>()), 
  codec_(std::make_unique<ClientCodec>(context, std::move(endpoint), ready_msgs_))
{ }

RoboClient::~RoboClient()
{
  codec_->closeConnection();
}

std::pair<std::shared_ptr<std::map<std::string, std::string>>, std::shared_ptr<docker_cpp::ImageList>>
RoboClient::getRoboContainersInfo()
{
  Message msg;
  msg.type = "Request";
  msg.command = "getRoboContainerInfo";
  msg.data = "";

  codec_->encode(msg);

  std::shared_ptr<Message> resp = ready_msgs_->wait_pop_back();

  size_t delim = resp->data.find(" ");
  std::string tag_map = resp->data.substr(0, delim);
  std::string image_info_list = resp->data.substr(delim + 1);

  json m = json::parse(tag_map);
  json im = json::parse(image_info_list);
  return {std::make_shared<std::map<std::string, std::string>>(m.get<std::map<std::string, std::string>>()),
          std::make_shared<docker_cpp::ImageList>(im.get<std::vector<docker_cpp::ImageInfo>>())};
}

bool RoboClient::startContainer(std::string&& id,  MContainerSetting settings)
{
  Message msg;
  msg.type = "Request";
  msg.command = "execRoboContainer";

	docker_cpp::CreateConfig conf;

	conf.Image = id.substr(7);
	conf.Tty = settings.tty_;
  conf.User = "docker_" + settings.container_username_;
	conf.OpenStdin = true;
  conf.NetworkDisabled = false;
  conf.Entrypoint.push_back("bash");
  conf.Entrypoint.push_back("-c");
  std::string subcommand1 = "source /opt/ros/humble/setup.bash";
  std::string subcommand4 = " && export ROS_DISCOVERY_SERVER=127.0.0.1:11811";
  std::string subcommand2 = " && set -a && source /home/docker_" + settings.container_username_ + "/launch/up.env && set +a";
  std::string subcommand3 = " && ros2 launch /home/docker_" + settings.container_username_ + "/launch/";
  std::string command = subcommand1 + subcommand4 + subcommand3 + settings.launch_ + ".launch.py " + settings.arguments_;
  conf.Cmd.push_back(command);
  conf.hostConfig.networkMode = "host";
  conf.hostConfig.autoRemove = false;
  conf.hostConfig.privileged = true;

  json j = conf;

  msg.data = settings.container_username_ + " " + settings.launch_dir_ + " " + settings.name_ + " " + j.dump();

  codec_->encode(msg);

  std::shared_ptr<Message> resp = ready_msgs_->wait_pop_back();

  return resp->data == "Success";
}

bool RoboClient::stopContainer(std::string&& id)
{
  Message msg;
  msg.type = "Request";
  msg.command = "stopRoboContainer";
  msg.data = std::move(id);

  codec_->encode(msg);

  std::shared_ptr<Message> resp = ready_msgs_->wait_pop_back();

  return resp->data == "Success";
}

std::shared_ptr<docker_cpp::ContainerList> RoboClient::getExecutedContainers()
{
  Message msg;
  msg.type = "Request";
  msg.command = "getExecutedRoboContainer";
  msg.data = "";

  codec_->encode(msg);

  std::shared_ptr<Message> resp = ready_msgs_->wait_pop_back();

  json j = json::parse(resp->data);
  return std::make_shared<docker_cpp::ContainerList>(j.get<std::vector<docker_cpp::ContainerInfo>>());
}

bool RoboClient::createDDSInstance(int id)
{
  Message msg;
  msg.type = "Request";
  msg.command = "execDDSServer";
  msg.data = std::to_string(id);

  codec_->encode(msg);

  std::shared_ptr<Message> resp = ready_msgs_->wait_pop_back();

  return resp->data == "Success";
}

bool RoboClient::deleteRoboImage(std::string&& id)
{
  Message msg;
  msg.type = "Request";
  msg.command = "deleteRoboImage";
  msg.data = std::move(id);

  codec_->encode(msg);

  std::shared_ptr<Message> resp = ready_msgs_->wait_pop_back();

  return resp->data == "Success";
}

bool RoboClient::buildRoboImage(std::string&& name, std::string&& target, std::string&& username)
{
  Message msg;
  msg.type = "Request";
  msg.command = "buildRoboImage";
  msg.data = std::move(name);
  msg.data += " ";
  msg.data += std::move(target);
  msg.data += " "; 
  msg.data += std::move(username);

  codec_->encode(msg);

  std::shared_ptr<Message> resp = ready_msgs_->wait_pop_back();

  return resp->data == "Success";
}

bool RoboClient::getRoboLog(std::string&& id, std::string& log)
{
  Message msg;
  msg.type = "Request";
  msg.command = "getRoboLog";
  msg.data = std::move(id);

  codec_->encode(msg);

  std::shared_ptr<Message> resp = ready_msgs_->wait_pop_back();

  size_t delim = resp->data.find(" ");
  std::string result = resp->data.substr(0, delim);
  log = resp->data.substr(delim + 1);

  return result == "Success";
}

RoboClientCreator::RoboClientCreator()
: context_(std::make_shared<boost::asio::io_context>()),
  work_guard_(std::make_shared<work_guard_type>(boost::asio::make_work_guard(*context_))),
  resolver_(std::make_shared<tcp::resolver>(*context_))
{ }

std::shared_ptr<RoboClient>
RoboClientCreator::createRoboClient(std::string& ip, std::string& port)
{
  auto&& endpoint = resolver_->resolve(ip.c_str(), port.c_str());

  return std::make_shared<RoboClient>(context_, std::move(endpoint));
}

std::shared_ptr<boost::asio::io_context>
RoboClientCreator::getContext()
{
  return context_;
}

std::shared_ptr<RoboClientCreator::work_guard_type>
RoboClientCreator::getWorkGuard()
{
  return work_guard_;
}