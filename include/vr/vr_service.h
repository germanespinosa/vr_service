#pragma once
#include <tcp_messages.h>
#include <vr/vr_agent_state.h>
#include <vr/vr_messages.h>

namespace vr {
    struct Vr_service : tcp_messages::Message_service {
        Routes (
                Add_route("agent_state", set_agent_state, Agent_state);
                Add_route("start_episode", start_episode, Vr_start_episode);
                Add_route("start_experiment", start_episode, Vr_start_episode);
                Add_route("finish_experiment", start_episode, Vr_start_episode);
                )
        static void set_agent_state (const Agent_state &);
        static void start_episode (const Vr_start_episode &);
        static void start_tracking_service(int port);
        static int get_port();
    };
}