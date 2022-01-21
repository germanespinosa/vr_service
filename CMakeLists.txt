cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(vr_service
        VERSION 2019.1.0
        DESCRIPTION "connects to unreal and interface with experiment services"
        LANGUAGES CXX)

find_package(Dependencies)

install_dependency(https://github.com/germanespinosa/tcp_messages Tcp_messages)
install_dependency(https://github.com/germanespinosa/agent_tracking Agent_tracking)
install_dependency(https://github.com/germanespinosa/cellworld Cellworld)

string(APPEND CMAKE_CXX_FLAGS " -fno-strict-aliasing -pthread ")

include_directories(include)

set(vr_service_files
     src/vr_service.cpp
     src/vr_agent_state.cpp)

add_library(vr_service ${vr_service_files})
target_link_libraries( vr_service
        LINK_PUBLIC
        cellworld
        agent_tracking )

add_executable(vr_server src/tools/server.cpp)

target_link_libraries(vr_server vr_service)
