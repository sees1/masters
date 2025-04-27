cmake_minimum_required(VERSION 3.11)
project(masters_server)

add_compile_options(-std=c++17 -Wpedantic -Wall)

option(MASTERS_BUILD_SERVER "Build server" ON)
option(MASTERS_BUILD_CLIENT "Build net client + gui" ON)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)


find_package(Boost COMPONENTS system)

# docker engine dependency
find_package(ASL REQUIRED)
find_package(docker_cpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(
  include
  ${docker_cpp_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${nlohmann_json_INCLUDE_DIRS}
)

# add_executable(chat_client 
  # src/client/client.cpp)

# add_executable(chat_server 
  # src/server/server.cpp)


# target_link_libraries(chat_client ${Boost_LIBRARIES} pthread)
# target_link_libraries(chat_server ${Boost_LIBRARIES} pthread)

qt5_add_resources(QT_RESOURCE_FILES include/client/gui/resources/resource.qrc)

qt5_wrap_cpp(main_window_lib include/client/gui/main_window.hpp
                             include/client/gui/server_widget.hpp
                             include/client/gui/server_credental_box.hpp
                             include/client/gui/server_container_create_box.hpp
                             include/client/gui/server_error_box.hpp)

add_executable(main_window ${main_window_lib} 
                            src/client/main.cpp
                            src/client/client.cpp
                            src/utils/NetCodec/client_codec.cpp
                            src/utils/NetCodec/codec.cpp
                            src/utils/NetCodec/decoder.cpp
                            src/utils/NetCodec/encoder.cpp
                            src/utils/message.cpp)

add_executable(server src/server/main.cpp
                      src/server/message_handler.cpp
                      src/server/server.cpp
                      src/utils/NetCodec/server_codec.cpp
                      src/utils/NetCodec/codec.cpp
                      src/utils/NetCodec/decoder.cpp
                      src/utils/NetCodec/encoder.cpp
                      src/utils/message.cpp)

target_link_libraries(main_window PRIVATE Qt5::Widgets 
                                          nlohmann_json::nlohmann_json 
                                          ${Boost_LIBRARIES}
                                          pthread)

target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json
                                     ${Boost_LIBRARIES}
                                     docker
                                     asls
                                     pthread)

install(TARGETS
  # chat_client chat_server
  main_window server
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)