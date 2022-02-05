#include <catch.h>
#include <vr.h>
#include <tcp_messages.h>
#include <cell_world.h>


using namespace tcp_messages;
using namespace vr;
using namespace std;
using namespace cell_world;


TEST_CASE("service_test") {
    Message_client client;
    client.connect("127.0.0.1", Vr_service::get_port());
    auto response = client.send_request(Message("get_world_implementation"),0).get_body<World_implementation>();

    cout << response << endl;
}