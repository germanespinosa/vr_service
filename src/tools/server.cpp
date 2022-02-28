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
    controller::Agent_operational_limits limits;
    limits.load("../config/ghost_operational_limits.json");

    Vr_server vr_server;
    vr_server.ghost_min_distance = 300;

    if (!vr_server.experiment_server.start(Experiment_service::get_port())){
        cout << "Failed to start experiment service" << endl;
        exit(1);
    }

    auto configuration = Resources::from("world_configuration").key("hexagonal").get_resource<World_configuration>();
    auto implementation = Resources::from("world_implementation").key("hexagonal").key("vr").get_resource<World_implementation>();
    vr_server.tracking_space =  Resources::from("world_implementation").key("hexagonal").key("canonical").get_resource<World_implementation>().space;
    auto capture_parameters = Resources::from("capture_parameters").key("default").get_resource<Capture_parameters>();
    auto peeking_parameters = Resources::from("peeking_parameters").key("default").get_resource<Peeking_parameters>();


    vr_server.set_world(configuration, implementation);
    vr_server.ghost_min_distance = 300;
    vr_server.start(Vr_service::get_port());
    auto &world = vr_server.world;
    auto cells = world.create_cell_group();
    Location_visibility visibility(cells, configuration.cell_shape, implementation.cell_transformation);
    Capture capture(capture_parameters, world);
    Peeking peeking(peeking_parameters, world);


    auto &controller_experiment_client = vr_server.experiment_server.create_local_client<Controller_server::Controller_experiment_client>();
    auto &controller_tracking_service = vr_server.tracking_server.create_local_client<Controller_server::Controller_tracking_client>(
            visibility,
            float(90),
            capture,
            peeking,
            "predator",
            "prey");

    Ghost ghost(limits, vr_server);

    Controller_server controller("../config/pid.json", ghost, controller_tracking_service, controller_experiment_client);
    controller.start(Controller_service::get_port());

    cout << "server running "<< endl;
    vr_server.join();
}
