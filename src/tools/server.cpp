#include <cell_world.h>
#include <vr/vr_service.h>
#include <iostream>
#include <agent_tracking.h>
#include <params_cpp.h>
#include <experiment.h>
#include <vr/ghost.h>

using namespace controller;
using namespace std;
using namespace tcp_messages;
using namespace vr;
using namespace agent_tracking;
using namespace params_cpp;
using namespace experiment;
using namespace cell_world;

int main(int argc, char **argv){
    Experiment_server experiment_server;
    experiment_server.start(Experiment_service::get_port());
    controller::Agent_operational_limits limits;
    limits.load("../config/ghost_operational_limits.json");
    Vr_server server;
    Vr_service::start_tracking_service(Tracking_service::get_port());
    Experiment_service::set_tracking_service_ip("127.0.0.1");
    Vr_service::connect_experiment_service("127.0.0.1");
    auto configuration = Resources::from("world_configuration").key("hexagonal").get_resource<World_configuration>();
    auto implementation = Resources::from("world_implementation").key("hexagonal").key("vr").get_resource<World_implementation>();
    auto tracking_space =  Resources::from("world_implementation").key("hexagonal").key("canonical").get_resource<World_implementation>().space;
    Vr_service::set_tracking_space(tracking_space);
    auto world = World(configuration,implementation);
    Vr_service::set_world(configuration, implementation);
    auto map = Map(world.create_cell_group());
    Vr_service::set_prey_start_location(map[Coordinates(-20,0)].location);
    Vr_service::set_ghost_min_distance(300);
    server.start(Vr_service::get_port());

    Ghost ghost(limits, server);
    Controller_server controller("../config/pid.json", ghost, "127.0.0.1", "127.0.0.1");
    controller.start(Controller_service::get_port());

    cout << "server running "<< endl;
    server.join();
}
