cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(vr_service
        VERSION 2019.1.0
        DESCRIPTION "connects to unreal and interface with experiment services"
        LANGUAGES CXX)

####
#### DEPENDENCIES
####

find_package (Dependencies QUIET)

if (NOT ${Dependencies_FOUND})
    if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/DependenciesConfig.cmake)
        file(DOWNLOAD https://raw.githubusercontent.com/germanespinosa/dependencies/main/DependenciesConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/DependenciesConfig.cmake)
    endif()
    set(Dependencies_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    find_package (Dependencies REQUIRED)
endif()

####
#### END DEPENDENCIES
####

install_git_dependency(Cellworld https://github.com/germanespinosa/cellworld
        CMAKE_PROJECT
        IMPORT_TARGETS cellworld json-cpp)
install_git_dependency(Easy-tcp https://github.com/germanespinosa/easy-tcp
        CMAKE_PROJECT
        IMPORT_TARGETS easy-tcp)
install_git_dependency(Tcp_messages https://github.com/germanespinosa/tcp-messages
        CMAKE_PROJECT
        IMPORT_TARGETS tcp_messages)
install_git_dependency(Agent_tracking https://github.com/germanespinosa/agent_tracking
        CMAKE_PROJECT
        IMPORT_TARGETS agent_tracking)
install_git_dependency(Experiment_service https://github.com/germanespinosa/experiment_service
        CMAKE_PROJECT
        IMPORT_TARGETS experiment_service)

install_git_dependency(Params-cpp https://github.com/germanespinosa/params-cpp
        CMAKE_PROJECT
        IMPORT_TARGETS params-cpp)

install_git_dependency(Controller https://github.com/germanespinosa/cellworld_pid_controller
        CMAKE_PROJECT
        IMPORT_TARGETS controller)

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
#install_dependency(https://github.com/germanespinosa/catch CatchTests)
#test_library(vr_service catchtests/vr_service_tests.cpp)
