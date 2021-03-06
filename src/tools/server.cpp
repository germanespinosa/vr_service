#include <cell_world.h>
#include <vr/vr_service.h>
#include <iostream>
#include <vr/ghost.h>

using namespace controller;
using namespace std;
using namespace tcp_messages;
using namespace vr;
using namespace agent_tracking;
using namespace experiment;
using namespace cell_world;

int main(int argc, char **argv){
    Experiment_service::set_logs_folder("experiments/");
    Controller_service::set_logs_folder("controller/");
    auto configuration = World_configuration::get_from_parameters_name("hexagonal");
    auto vr_implementation = World_implementation::get_from_parameters_name("hexagonal", "vr");
    auto canonical_implementation = World_implementation::get_from_parameters_name("hexagonal", "canonical");
    auto vr_world = World(configuration, vr_implementation);
    vr_implementation.scale(Scale(-1,1));
    Vr_server vr_server(configuration,vr_implementation, canonical_implementation);
    vr_server.ghost_min_distance = .25;
    auto canonical_world = World(configuration,canonical_implementation);
    auto visibility = canonical_world.create_location_visibility();
    vr_server.start(Vr_service::get_port());
    cout << "server running "<< endl;
    vr_server.join();
}
