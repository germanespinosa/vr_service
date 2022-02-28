#pragma once
#include <cell_world.h>
#include <tcp_messages.h>
#include <vr/vr_agent_state.h>
#include <vr/vr_messages.h>
#include <experiment.h>
#include <agent_tracking.h>

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
            void on_experiment_started(const experiment::Start_experiment_response  &experiment) override {
                vr_server->last_experiment_started.experiment_name = experiment.experiment_name;
                vr_server->last_experiment_started.occlusions = cell_world::Resources::from("cell_group").key(experiment.world.world_configuration).key(experiment.world.occlusions).key("occlusions").get_resource<cell_world::Cell_group_builder>();
                vr_server->last_experiment_started.is_active = true;
                vr_server->world.set_occlusions(vr_server->last_experiment_started.occlusions);
                auto free_cells = vr_server->world.create_cell_group().free_cells();
                vr_server->last_experiment_started.ghost_spawn_locations.clear();
                for (auto &cell:free_cells){
                    auto location = cell.get().location;
                    if (vr_server->prey_start_location.dist(location)>vr_server->ghost_min_distance)
                        vr_server->last_experiment_started.ghost_spawn_locations.push_back(location);
                }
                vr_server->pending_participant = true;
                std::cout << "Experiment " << experiment.experiment_name << " started, waiting for participant" << std::endl;
            };

            void on_experiment_finished(const std::string &experiment_name) override {
                std::map<int, Vr_server::Active_experiment_data>::iterator it;
                for(it=vr_server->active_experiments.begin(); it!=vr_server->active_experiments.end(); ++it){
                    if (it->second.experiment_name == experiment_name) {
                        vr_server->active_experiments.erase(it->first);
                        std::cout << "Closing experiment " << experiment_name << " for participant " << it->first << std::endl;
                    }
                }
            };
            Vr_server *vr_server;
        };

        struct Active_experiment_data {
            std::string experiment_name;
            cell_world::Cell_group_builder occlusions;
            cell_world::Location_list ghost_spawn_locations;
            bool is_active;
        };

        Vr_server();
        cell_world::World_implementation get_world_implementation() const;
        Vr_start_episode_response start_episode (const Vr_start_episode_request &);
        Vr_finish_episode_response finish_episode (const Vr_finish_episode_request &);
        void set_ghost_movement(float forward, float rotation);
        void capture();
        void on_new_connection(Vr_service &new_connection) override;
        void set_agent_state (const Agent_state &);
        void set_world(const cell_world::World_configuration &, const cell_world::World_implementation &);

        experiment::Experiment_server experiment_server;

        agent_tracking::Tracking_server tracking_server;
        Vr_experiment_client &experiment_client;

        Active_experiment_data last_experiment_started;
        std::map<int, Active_experiment_data> active_experiments;
        cell_world::World_configuration configuration;
        cell_world::World_implementation implementation;
        cell_world::World world;
        cell_world::Space tracking_space;
        cell_world::Scale vr_scale{-1, 1};
        bool pending_participant;
        cell_world::Location prey_start_location;
        float forward_speed = .5;
        float rotation_speed = .5;
        float ghost_min_distance = 0;
    };

}