#include <cell_world.h>
#include <vr/vr_service.h>
#include <iostream>
#include <agent_tracking.h>
#include <params_cpp.h>
#include <experiment.h>


using namespace std;
using namespace tcp_messages;
using namespace vr;
using namespace agent_tracking;
using namespace params_cpp;
using namespace experiment;
using namespace cell_world;

int main(int argc, char **argv){
    auto p = Parser(argc, argv);
    auto rotation_speed = stof(p.get( Key("-rotation_speed","-rotation","-r"), "1.0"));
    auto forward_speed = stof(p.get( Key("-forward_speed","-forward","-f"), "1.0"));
    auto experiment_service_ip = p.get( Key("-experiment_service_ip","-experiment","-e"), "127.0.0.1");
    Experiment_server experiment_server;
    experiment_server.start(Experiment_service::get_port());
    Vr_server server;
    Vr_service::start_tracking_service(Tracking_service::get_port());
    Experiment_service::set_tracking_service_ip("127.0.0.1");
    Vr_service::connect_experiment_service(experiment_service_ip);
    Vr_service::start_ghost(4500);
    Vr_service::set_ghost_forward_speed(forward_speed);
    Vr_service::set_ghost_rotation_speed(rotation_speed);
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
    cout << "server running "<< endl;
    server.join();
}
