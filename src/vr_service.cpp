#include <vr/vr_service.h>
#include <agent_tracking/tracking_service.h>
#include <tcp_messages/message_server.h>
#include <map>

using namespace std;
using namespace agent_tracking;

namespace vr {
    map<int, std::string> active_experiment_ids;
    map<std::string, Agent_state> agent_states;
    Tracking_server vr_tracking;
    unsigned int frame_count = 0;
    cell_world::Timer time_stamp;

    void Vr_service::set_agent_state(const Agent_state &agent_state) {
        if (agent_states.contains(agent_state.agent_name) && agent_state == agent_states[agent_state.agent_name])
            return;
        agent_states[agent_state.agent_name] = agent_state;
        cell_world::Step step;
        step.agent_name = agent_state.agent_name;
        step.frame = ++frame_count;
        step.location = agent_state.location.to_location();
        step.rotation = agent_state.rotation.yaw;
        step.time_stamp = time_stamp.to_seconds();
        agent_tracking::Tracking_service::send_step(step);
        cout << agent_state << endl;
    }

    void Vr_service::start_episode(const Vr_start_episode &) {
        time_stamp.reset();
    }

    int Vr_service::get_port() {
        string port_str(std::getenv("CELLWORLD_VR_SERVICE_PORT") ? std::getenv("CELLWORLD_VR_SERVICE_PORT") : "4540");
        return atoi(port_str.c_str());
    }

    void Vr_service::start_tracking_service(int port) {
        vr_tracking.start(port);
    }
}