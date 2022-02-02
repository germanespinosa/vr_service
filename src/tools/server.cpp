#include <vr/vr_service.h>
#include <iostream>
#include <agent_tracking.h>
#include <params_cpp.h>


using namespace std;
using namespace tcp_messages;
using namespace vr;
using namespace agent_tracking;
using namespace params_cpp;

int main(int argc, char **argv){
    auto p = Parser(argc, argv);
    auto rotation_speed = stof(p.get( Key("-rotation_speed","-rotation","-r"), "1.0"));
    auto forward_speed = stof(p.get( Key("-forward_speed","-forward","-f"), "1.0"));
    auto experiment_service_ip = p.get( Key("-experiment_service_ip","-experiment","-e"), "127.0.0.1");
    Vr_server server;
    Vr_service::start_tracking_service(Tracking_service::get_port());
    Vr_service::connect_experiment_service(experiment_service_ip);
    Vr_service::set_ghost_forward_speed(forward_speed);
    Vr_service::set_ghost_rotation_speed(rotation_speed);
    Vr_service::set_configuration("hexagonal");
    Vr_service::set_implementation("hexagonal.vr");
    server.start(Vr_service::get_port());
    cout << "server running "<< endl;
    server.join();
}
