#include "server/message_handler.hpp"
#include <boost/process.hpp>
#include <fstream>

#include <limits.h>
#include <unistd.h>

using nlohmann::json_abi_v3_11_3::json;

RequestHandler::RequestHandler()
: docker_engine_ptr_(std::make_shared<Docker<ASLHttp>>("http://127.0.0.1:2375"))
{

  // TODO: refactor error system here
  if (!docker_engine_ptr_->checkConnection())
    throw std::runtime_error("Can't connect to docker engine web api on server!");

  command_to_exec_.insert(std::make_pair("getRoboContainerInfo", std::make_shared<RoboContainerInfoExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("execRoboContainer", std::make_shared<RoboContainerExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("getExecutedRoboContainer", std::make_shared<RoboStartedContainerInfoExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("execDDSServer", std::make_shared<RoboStartDDSExec>(docker_engine_ptr_)));
}

std::shared_ptr<Message> RequestHandler::handleRequest(std::shared_ptr<Message> reciv_msg)
{
  std::shared_ptr<Executor> exec = chooseExecutor(reciv_msg->command);

  return exec->execute(reciv_msg);
}

std::shared_ptr<Executor> RequestHandler::chooseExecutor(std::string& command)
{
  auto&& exec_it = command_to_exec_.find(command);

  if(exec_it != command_to_exec_.end())
    return exec_it->second;
  // else
    // TODO: add error logic
}

std::shared_ptr<Message> RoboStartDDSExec::execute(std::shared_ptr<Message> msg) {
  std::string id = msg->data;
  std::string command = "fastdds discovery --server-id " + id;

  std::shared_ptr<Message> m = std::make_shared<Message>();
  m->type = "Response";

  disc_serv_ = bp::child("/usr/bin/bash", "-c", "fastdds", "discovery", "--server-id", "0");

  if(disc_serv_)
    m->data = "Success";
  else
    m->data = "Failure";

  return m;
}

RoboStartDDSExec::~RoboStartDDSExec() {
  disc_serv_->terminate();
  disc_serv_->wait();
}
    
std::shared_ptr<Message> RoboContainerInfoExec::execute(std::shared_ptr<Message> msg)
{
  docker_cpp::ImageList images;
  docker_cpp::DockerError err = docker_engine_copy_->imageList(images, true);

  // TODO: refactor error system here
  if (err.isError()) { 
    std::cout << "Error creating image images\n";
    std::cout << err;
    std::abort();
  }
  else
  {
    std::cout << "Image successfully created...\n";
  }

  std::string imageId;

  docker_cpp::ImageList roboImages;

  std::copy_if(images.begin(), images.end(), roboImages.begin(), 
    [](auto&& img)
    {
      return std::find(img.repoTags.begin(), img.repoTags.end(), "ros2") != img.repoTags.end();
    }
  );

  char path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);

  std::string path_s;
  if (len != -1) {
    path[len] = '\0';
    path_s = path;
    std::cout << "Current exec file path: " << path << std::endl;
  }
  else
  {
    std::cout << "Error while try take current exec file path" << std::endl;
  }

  std::vector<std::string> tokens;
  std::stringstream ss(path_s.substr(1));
  std::string token;
  while (std::getline(ss, token, '/')) {
      if (!token.empty()) {
          tokens.push_back(token);
      }
  }

  if(tokens.size() != 0)
    tokens.erase(tokens.end() - 2, tokens.end());

  std::string docker_dir_path = "/";
  std::for_each(tokens.begin(), tokens.end(), 
    [&docker_dir_path](auto&& token)
    {
      docker_dir_path += token + "/";
    }
  );
  std::string docker_build_dir_path = docker_dir_path + "docker/build/";

  std::cout << docker_build_dir_path << std::endl;

  std::ifstream index (docker_build_dir_path + "index.txt");

  std::map<std::string, std::string> tag_target_string_map;

  if (!index.is_open()) {
    std::cout << "Can't open index.txt!";
  }
  else 
  {
    std::string line;
    while(std::getline(index, line)) {
      size_t delim_pos = line.find(' ');
      std::string tagname = line.substr(0, delim_pos);
      std::string target = line.substr(delim_pos + 1);
      tag_target_string_map.insert(std::make_pair(tagname, target));
    }
  }

  json tag_map = tag_target_string_map;

  json j = roboImages;

  std::shared_ptr<Message> m = std::make_shared<Message>();
  m->type = "Response";
  m->data = tag_map.dump();
  m->data += " ";
  m->data += j.dump(-1, ' ', false, json::error_handler_t::ignore);

  return m;
}

std::shared_ptr<Message> RoboContainerExec::execute(std::shared_ptr<Message> msg)
{
  std::size_t name_end = msg->data.find(" ");
  std::string name = msg->data.substr(0, name_end);
  std::string json_str = msg->data.substr(name_end+1);

  json j = json::parse(json_str);
  std::shared_ptr<docker_cpp::CreateConfig> conf_ptr = std::make_shared<docker_cpp::CreateConfig>(j.get<docker_cpp::CreateConfig>());
  
	docker_cpp::DockerError err_create = docker_engine_copy_->createContainer(*conf_ptr, name);
  docker_cpp::DockerError err_start = docker_engine_copy_->containerStart(name);

  std::shared_ptr<Message> m = std::make_shared<Message>();

  m->type = "Response";
  
  // TODO: refactor error system here
  if (err_create.isError() || err_start.isError()) { 
    std::cout << "Error start container\n";
    m->data = "Failure";
  }
  else
  {
    m->data = "Success";
    std::cout << "Container successfully started...\n";
  }

  return m;
}

std::shared_ptr<Message> RoboStartedContainerInfoExec::execute(std::shared_ptr<Message> msg)
{
  docker_cpp::ContainerList container_list;
  docker_cpp::DockerError err = docker_engine_copy_->containerList(container_list);

  std::shared_ptr<Message> m = std::make_shared<Message>();
  
  // TODO: refactor error system here
  if (err.isError()) { 
    std::cout << "Error list executed containers\n";
    std::cout << err;
  }
  else
  {
    std::cout << "List of executed container successfully process...\n";
  }

  json j = container_list;

  m->type = "Response";
  m->data = j.dump(-1, ' ', false, json::error_handler_t::ignore);
  return m;
}