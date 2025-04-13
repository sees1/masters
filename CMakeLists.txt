cmake_minimum_required(VERSION 3.11)
project(masters_server)

add_compile_options(-std=c++17 -Wpedantic -Wall)

find_package(Boost COMPONENTS system)

include_directories(
  include
  ${Boost_INCLUDE_DIRS}
)

add_executable(chat_client 
  src/client.cpp)

add_executable(chat_server 
  src/server.cpp)

target_link_libraries(chat_client ${Boost_LIBRARIES} pthread)
target_link_libraries(chat_server ${Boost_LIBRARIES} pthread)

install(TARGETS
  chat_client chat_server
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)