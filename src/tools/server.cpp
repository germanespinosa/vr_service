#include <vr/vr_service.h>
#include <tcp_messages.h>
#include <iostream>
#include <agent_tracking.h>


using namespace std;
using namespace tcp_messages;
using namespace vr;
using namespace agent_tracking;

int main(int argc, char **argv){
    Message_server<Vr_service> server;
    server.start(Vr_service::get_port());
    Vr_service::start_tracking_service(Tracking_service::get_port());
    cout << "server running "<< endl;
    server.join();
}
