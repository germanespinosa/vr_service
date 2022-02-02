#include <catch.h>
#include <vr.h>
#include <tcp_messages.h>


using namespace tcp_messages;
using namespace vr;
using namespace std;

TEST_CASE("service_test") {
    Message_client client;
    client.connect("127.0.0.1", Vr_service::get_port());
    auto response = client.send_request(Message("get_world_implementation"),0);
    cout << response << endl;
}