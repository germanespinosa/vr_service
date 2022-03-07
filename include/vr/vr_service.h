#pragma once
#include <cell_world.h>
#include <tcp_messages.h>
#include <vr/vr_agent_state.h>
#include <vr/vr_messages.h>
#include <experiment.h>
#include <agent_tracking.h>
#include <controller.h>
#include <vr/ghost.h>

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
        void set_agent_state (const Agent_state &);
        cell_world::World_implementation get_world_implementation();
        Vr_start_episode_response start_episode (const Vr_start_episode_request &);
        Vr_finish_episode_response finish_episode (const Vr_finish_episode_request &);
        static int get_port();
    };


    struct Vr_server : tcp_messages::Message_server<Vr_service> {
        struct Vr_experiment_client : experiment::Experiment_client {
            void on_experiment_started(const experiment::Start_experiment_response  &experiment) override;
            void on_experiment_finished(const std::string &experiment_name) override;
            void on_behavior_set(int behavior) override;
            void on_capture(int frame) override;
            Vr_server *vr_server;
        };
        struct Active_experiment_data {
            std::string experiment_name;
            cell_world::Cell_group_builder occlusions;
            cell_world::Location_list ghost_spawn_locations;
            bool is_active;
        };

        Vr_server(const cell_world::World_configuration &, const cell_world::World_implementation &, const cell_world::World_implementation &);
        void on_experiment_started(const experiment::Start_experiment_response &experiment);
        [[nodiscard]] cell_world::World_implementation get_world_implementation() const;
        Vr_start_episode_response start_episode (const Vr_start_episode_request &);
        Vr_finish_episode_response finish_episode (const Vr_finish_episode_request &);
        void set_ghost_movement(float forward, float rotation);
        void new_capture();
        void on_new_connection(Vr_service &new_connection) override;
        void set_agent_state (const Agent_state &);
        void set_occlusions(const cell_world::Cell_group_builder &occlusions);
        void chasing();
        void exploring();

        experiment::Experiment_server experiment_server;
        agent_tracking::Tracking_server tracking_server;

        cell_world::World_configuration configuration;
        cell_world::World_implementation vr_implementation;
        cell_world::World vr_world;
        cell_world::World_implementation canonical_implementation;
        cell_world::World canonical_world;
        cell_world::Location_visibility canonical_visibility;

        cell_world::Capture capture;
        cell_world::Peeking peeking;

        experiment::Experiment_tracking_client &experiment_tracking_client;


        controller::Agent_operational_limits ghost_operational_limits;
        Ghost ghost;
        controller::Controller_server::Controller_tracking_client &controller_tracking_client;
        controller::Controller_server controller_server;

        Vr_experiment_client &experiment_client;
        Active_experiment_data last_experiment_started;
        std::map<int, Active_experiment_data> active_experiments;
        cell_world::Scale vr_scale{-1, 1};
        bool pending_participant;
        cell_world::Location prey_start_location {0,.5};
        float forward_speed = .5;
        float rotation_speed = .5;
        float ghost_min_distance = 0;
        bool episode_in_progress= false;
    };

}