cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(vr_service
        VERSION 2019.1.0
        DESCRIPTION "connects to unreal and interface with experiment services"
        LANGUAGES CXX)

find_package(Dependencies)

install_dependency(https://github.com/germanespinosa/tcp-messages Tcp_messages)
install_dependency(https://github.com/germanespinosa/params-cpp Params-cpp)
install_dependency(https://github.com/germanespinosa/agent_tracking Agent_tracking)
install_dependency(https://github.com/germanespinosa/cellworld Cellworld)
install_dependency(https://github.com/germanespinosa/cellworld_controller Controller)
install_dependency(https://github.com/germanespinosa/experiment_service Experiment_service)

string(APPEND CMAKE_CXX_FLAGS " -fno-strict-aliasing -pthread ")

include_directories(include)

include_directories(dependencies/params-cpp/include)

set(vr_service_files
     src/vr_service.cpp
     src/vr_agent_state.cpp
     src/ghost.cpp )

add_library(vr_service ${vr_service_files})
target_link_libraries( vr_service
        LINK_PUBLIC
        cellworld
        agent_tracking
        experiment_service
        params-cpp
        controller)

add_executable(vr_server src/tools/server.cpp)

target_link_libraries(vr_server vr_service)

###
### TESTS
###
install_dependency(https://github.com/germanespinosa/catch CatchTests)
test_library(vr_service catchtests/vr_service_tests.cpp)
