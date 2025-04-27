#include <iostream>
#include <cstdlib>
#include <string>
#include <boost/process.hpp>
#include <optional>

namespace bp = boost::process;

int main() {

 std::optional<bp::child> disc_serv_ = bp::child("/usr/bin/bash", "-c", "fastdds", "discovery", "--server-id", "0");

 std::cout << disc_serv_->id() << std::endl;

 if(disc_serv_)
    std::cout << "Success" << std::endl;
 else
    std::cout << "Failure" << std::endl;

 disc_serv_->wait();

 return 0;
}
