#include "server/message_handler.hpp"
#include <boost/process.hpp>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <limits.h>
#include <unistd.h>

namespace fs = std::filesystem;
using nlohmann::json_abi_v3_11_3::json;

RequestHandler::RequestHandler()
: docker_engine_ptr_(std::make_shared<Docker<ASLHttp>>("http://127.0.0.1:2375"))
{
  // TODO: refactor error system here
  if (!docker_engine_ptr_->checkConnection())
    throw std::runtime_error("Can't connect to docker engine web api on server!");

  command_to_exec_.insert(std::make_pair("getRoboContainerInfo",     std::make_shared<RoboContainerInfoExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("getExecutedRoboContainer", std::make_shared<RoboStartedContainerInfoExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("getRoboLog",               std::make_shared<RoboContainerLogExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("execDDSServer",            std::make_shared<RoboStartDDSExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("execRoboContainer",        std::make_shared<RoboContainerExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("stopRoboContainer",        std::make_shared<RoboStopContainerExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("deleteRoboImage",          std::make_shared<RoboDeleteImageExec>(docker_engine_ptr_)));
  command_to_exec_.insert(std::make_pair("buildRoboImage",           std::make_shared<RoboBuildImageExec>(docker_engine_ptr_)));
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

std::shared_ptr<Message> RoboStartDDSExec::execute(std::shared_ptr<Message> msg)
{
  std::string id = msg->data;
  std::string command = "fastdds discovery --server-id " + id;

  std::shared_ptr<Message> m = std::make_shared<Message>();
  m->type = "Response";

  disc_serv_ = bp::child("/usr/bin/bash", "-c", command.c_str());

  const char* env_key = "ROS_DISCOVERY_SERVER";
  const char* env_value = "127.0.0.1:11811";

  std::string env_error;
  std::string fastdds_error;

  if (setenv(env_key, env_value, 1) != 0) {
    env_error = "Ошибка при установке переменной окружения";
  }
  if (!disc_serv_)
  {
    fastdds_error = "Ошибка при запуске fastdds!";
  }

  if(disc_serv_)
  {
    m->data = "Success";
  }
  else
  {
    m->data = "Failure";
    m->data += env_error.empty() ? "" : "|" + env_error;
    m->data += fastdds_error.empty() ? "" : "|" + fastdds_error;
  }

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

  std::copy_if(images.begin(), images.end(), std::back_inserter(roboImages), 
    [](auto&& img)
    {
      auto&& needed_tag = std::find_if(img.repoTags.begin(), img.repoTags.end(),
        [](auto&& tag)
        {
          size_t delim = tag.find(":");
          std::string real_tag = tag.substr(delim + 1);

          std::cout << "read robo tag: " << real_tag <<  std::endl;

          return real_tag == "ros2";
        }
      );
      
      return needed_tag != img.repoTags.end();
    }
  );
  
  std::cout << "Robo images size: " << roboImages.size() << std::endl;

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
  std::string docker_build_dir_path = docker_dir_path + "docker/build-context/";

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
  std::size_t container_usr_name_end = msg->data.find(" ");
  std::string container_usr_name = msg->data.substr(0, container_usr_name_end);
  std::string temp_str = msg->data.substr(container_usr_name_end + 1);
  size_t delim = temp_str.find(" ");
  std::string launch_dir = temp_str.substr(0, delim);
  temp_str = temp_str.substr(delim + 1);
  delim = temp_str.find(" ");
  std::string name = temp_str.substr(0, delim);
  std::string json_str = temp_str.substr(delim+1);

  json j = json::parse(json_str);
  std::shared_ptr<docker_cpp::CreateConfig> conf_ptr = std::make_shared<docker_cpp::CreateConfig>(j.get<docker_cpp::CreateConfig>());

  std::cout << "---------------------------" << std::endl;
  std::cout << "Current POST command in container: " << j.dump(' ') << std::endl;
  std::cout << "---------------------------" << std::endl;

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
  std::string docker_run_dir_path = docker_dir_path + "docker/run";

  fs::path run_dir_path = docker_run_dir_path.c_str();

  std::string rmw_path;
  std::string pkg_config_path;
  std::string pkg_launch_path;
  
  if(fs::exists(run_dir_path))
  {
    for(auto&& entry : fs::directory_iterator(run_dir_path))
    {
      if (entry.is_directory() && entry.path().filename() == "config")
      {
        for(auto& entry_inner : fs::directory_iterator(entry.path()))
        {
          if (entry_inner.is_directory() && entry_inner.path().filename() == launch_dir)
            pkg_config_path = entry_inner.path().c_str();
          else if (entry_inner.is_directory() && entry_inner.path().filename() == "rmw")
            rmw_path = entry_inner.path().c_str();
        }
      }
      else if (entry.is_directory() && entry.path().filename() == "launch")
      {
        for(auto&& entry_inner : fs::directory_iterator(entry.path()))
        {
          if (entry_inner.is_directory() && entry_inner.path().filename() == launch_dir)
            pkg_launch_path = entry_inner.path().c_str();
        }
      }
    }
  }

  std::cout << "-------------------------" << std::endl;
  std::cout << "Pkg Config path: " << pkg_config_path << std::endl;
  std::cout << "Pkg Launch path: " << pkg_launch_path << std::endl;
  std::cout << "Rmw path: " << rmw_path << std::endl;
  std::cout << "-------------------------" << std::endl;

  if (pkg_config_path.empty() || pkg_launch_path.empty())
    std::cout << "Ничего не найдено **** из папок";

  
  conf_ptr->hostConfig.binds.push_back(pkg_config_path + ":" + "/home/docker_" + container_usr_name + "/config");
  conf_ptr->hostConfig.binds.push_back(pkg_launch_path + ":" + "/home/docker_" + container_usr_name + "/launch");

	docker_cpp::DockerError err_create = docker_engine_copy_->createContainer(*conf_ptr, name);
  docker_cpp::DockerError err_start = docker_engine_copy_->containerStart(name);

  std::shared_ptr<Message> m = std::make_shared<Message>();

  m->type = "Response";
  
  // TODO: refactor error system here
  if (err_create.isError() || err_start.isError()) { 
    std::cout << "Error start container\n";
    std::cout << err_start << std::endl;
    m->data = "Failure";
  }
  else
  {
    m->data = "Success";
    std::cout << err_start << std::endl;
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

std::shared_ptr<Message> RoboDeleteImageExec::execute(std::shared_ptr<Message> msg)
{
  docker_cpp::DeletedImageList del_img_list;

  docker_cpp::DockerError err = docker_engine_copy_->imageRemove(msg->data, del_img_list, true);

  std::shared_ptr<Message> m = std::make_shared<Message>();

  m->type = "Response";

  // TODO: refactor error system here
  if (err.isError()) { 
    std::cout << "Error while delete image!\n";
    std::cout << err;
    m->data = "Failure";
  }
  else
  {
    std::cout << "Image successfully deleted...\n";
    m->data = "Success";
  }

  return m;
}

std::shared_ptr<Message> RoboBuildImageExec::execute(std::shared_ptr<Message> msg)
{
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

  std::string dockerfile_path = docker_dir_path + "docker/build-context/context.tar.gz";


  size_t delim = msg->data.find(" ");
  std::string name = msg->data.substr(0, delim);
  std::string temp = msg->data.substr(delim + 1);
  delim = temp.find(" ");

  
  std::string target = temp.substr(0, delim);
  std::string username = temp.substr(delim + 1);
  
  std::map<std::string, std::string> filter_map_;
  
  filter_map_.insert(std::make_pair("CONTAINER_NAME", username));

  std::string tag = name + ":ros2";

  std::cout << "Robo Server Build Image with tag: " << tag << std::endl;

  docker_cpp::DockerError err = docker_engine_copy_->imageBuild(dockerfile_path,
                                                                tag,
                                                                target,
                                                                filter_map_);

  std::shared_ptr<Message> m = std::make_shared<Message>();

  m->type = "Response";

  // TODO: refactor error system here
  if (err.isError()) { 
    std::cout << "Error while build image!\n";
    std::cout << err;
    m->data = "Faliure";
  }
  else
  {
    std::cout << "Build successfully ended...\n";
    m->data = "Success";
  }
                                                              
  return m;
}

std::shared_ptr<Message> RoboStopContainerExec::execute(std::shared_ptr<Message> msg)
{
  docker_cpp::DockerError err = docker_engine_copy_->containerStop(msg->data);

  std::shared_ptr<Message> m = std::make_shared<Message>();

  m->type = "Response";
  
  // TODO: refactor error system here
  if (err.isError()) { 
    std::cout << "Error stop container\n";
    std::cout << err << std::endl;
    m->data = "Failure";
  }
  else
  {
    m->data = "Success";
    std::cout << "Container successfully stoped...\n";
  }

  return m;
}

std::shared_ptr<Message> RoboContainerLogExec::execute(std::shared_ptr<Message> msg)
{
  std::string log;

  docker_cpp::DockerError err = docker_engine_copy_->containerLog(msg->data, log);

  std::shared_ptr<Message> m = std::make_shared<Message>();

  m->type = "Response";
  
  // TODO: refactor error system here
  if (err.isError()) { 
    std::cout << "Error get log from container\n";
    std::cout << err << std::endl;
    m->data = "Failure";
  }
  else
  {
    m->data = "Success";
    std::cout << "Log successfully getted...\n";
  }

  m->data += " ";
  m->data += log;

  return m;
}