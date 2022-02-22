#pragma once
#include <cell_world.h>
#include <tcp_messages.h>
#include <vr/vr_agent_state.h>
#include <vr/vr_messages.h>

namespace vr {
    struct Vr_server;

    struct Vr_service : tcp_messages::Message_service {
        Routes (
                Add_route_with_response("get_world_implementation", get_world_implementation);
                Add_route_with_response("start_episode", start_episode, Vr_start_episode_request);
                Add_route_with_response("finish_episode", finish_episode, Vr_finish_episode_request);
                Add_route("agent_state", set_agent_state, Agent_state);
                Allow_subscription();
                )
        static cell_world::World_implementation get_world_implementation();
        static void set_agent_state (const Agent_state &);
        static Vr_start_episode_response start_episode (const Vr_start_episode_request &);
        static Vr_finish_episode_response finish_episode (const Vr_finish_episode_request &);
        static void start_tracking_service(int port);
        static int get_port();
        static bool connect_experiment_service(const std::string  &ip);
        static void set_world(const cell_world::World_configuration &, const cell_world::World_implementation &);
        static void set_ghost_min_distance(float);
        static void set_prey_start_location(const cell_world::Location &);
        static void set_tracking_space(const cell_world::Space &);
    };


    struct Vr_server : tcp_messages::Message_server<Vr_service> {
        Vr_server();
        void set_ghost_movement(float forward, float rotation);
        void capture();
        void on_new_connection(Vr_service &new_connection) override;
    };
}