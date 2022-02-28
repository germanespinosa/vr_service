#include <vr/vr_service.h>
#include <experiment/experiment_client.h>
#include <easy_tcp.h>
#include <controller/agent.h>

using namespace std;
using namespace agent_tracking;
using namespace experiment;
using namespace cell_world;
using namespace controller;
using namespace tcp_messages;

namespace vr {

    void Vr_service::set_agent_state(const Agent_state &agent_state) {
        auto server = ((Vr_server *)_server);
        server->set_agent_state(agent_state);
    }

    int Vr_service::get_port() {
        string port_str(std::getenv("CELLWORLD_VR_SERVICE_PORT") ? std::getenv("CELLWORLD_VR_SERVICE_PORT") : "4580");
        return atoi(port_str.c_str());
    }

    Vr_finish_episode_response Vr_service::finish_episode(const Vr_finish_episode_request &parameters) {
        auto server = ((Vr_server *)_server);
        return server->finish_episode(parameters);
    }

    Vr_start_episode_response Vr_service::start_episode(const Vr_start_episode_request &parameters) {
        auto server = ((Vr_server *)_server);
        return server->start_episode(parameters);
    }

    cell_world::World_implementation Vr_service::get_world_implementation() {
        auto server = ((Vr_server *)_server);
        return server->get_world_implementation();
    }

    void Vr_server::set_world(const cell_world::World_configuration &new_configuration, const cell_world::World_implementation &new_implementation) {
        configuration = new_configuration;
        implementation = new_implementation;
        implementation.scale(vr_scale);
        world = World(configuration, implementation);
    }


    void Vr_server::set_ghost_movement(float forward, float rotation) {
        Vr_update_ghost_movement parameters;
        parameters.forward = forward;
        parameters.rotation = rotation;
        broadcast_subscribed(Message("update_ghost_movement",parameters));
    }

    void Vr_server::on_new_connection(Vr_service &new_connection) {
        cout << "New connection" << endl;
        Message_server::on_new_connection(new_connection);
    }

    void Vr_server::capture() {
        broadcast_subscribed(Message("capture"));
    }

    cell_world::World_implementation Vr_server::get_world_implementation() const {
        return implementation;
    }

    Vr_start_episode_response Vr_server::start_episode(const Vr_start_episode_request &parameters) {
        Vr_start_episode_response response;
        Active_experiment_data experiment;
        if (pending_participant) { // first episode of the new experiment
            experiment = last_experiment_started;
            cout << "Experiment " << experiment.experiment_name << "associated to participant " << parameters.participant_id;
            active_experiments[parameters.participant_id] = experiment;
            pending_participant = false;
        } else {  // if not new experiment
            if (active_experiments.contains(parameters.participant_id)) {
                experiment = active_experiments[parameters.participant_id];
            } else { // participant not registered
                cout << "No active experiment for participant "<< parameters.participant_id << endl;
                return response;
            }
        }
        cout << "Starting episode for experiment " << experiment.experiment_name <<  endl;
        experiment_client.start_episode(experiment.experiment_name);
        response.occlusions = experiment.occlusions;
        response.predator_spawn_location = experiment.ghost_spawn_locations[cell_world::Chance::dice(experiment.ghost_spawn_locations.size())];
        cout << response << endl;
        return response;
    }

    Vr_finish_episode_response Vr_server::finish_episode(const Vr_finish_episode_request &parameters) {
        Vr_finish_episode_response response;
        if (active_experiments.contains(parameters.participant_id)){
            thread async_finish_episode ( [this, parameters] () {
                auto &experiment = active_experiments[parameters.participant_id];
                experiment_client.finish_episode();
                experiment.is_active = experiment_client.is_active(experiment.experiment_name);
            });
            async_finish_episode.detach();
        }
        return response;
    }

    void Vr_server::set_agent_state(const Agent_state &agent_state) {
        cell_world::Step step;
        step.agent_name = agent_state.agent_name;
        step.frame = agent_state.frame;
        step.location = implementation.space.scale(agent_state.location.to_location(), Scale{-1,1});
        step.rotation = agent_state.rotation.yaw - 90;
        step.time_stamp = agent_state.time_stamp;
        auto converted = step.convert(implementation.space, tracking_space);
        tracking_server.send_step(converted);
    }

    Vr_server::Vr_server() : experiment_client(experiment_server.create_local_client<Vr_experiment_client>()){
        experiment_client.vr_server = this;
        if (!experiment_server.start(Experiment_service::get_port())){
            cout << "Failed to start experiment service" << endl;
            exit(1);
        }
        experiment_client.subscribe();
    }

    void Vr_server::Vr_experiment_client::on_experiment_started(const Start_experiment_response &experiment) {
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
    }

    void Vr_server::Vr_experiment_client::on_experiment_finished(const string &experiment_name) {
        std::map<int, Vr_server::Active_experiment_data>::iterator it;
        for(it=vr_server->active_experiments.begin(); it!=vr_server->active_experiments.end(); ++it){
            if (it->second.experiment_name == experiment_name) {
                vr_server->active_experiments.erase(it->first);
                std::cout << "Closing experiment " << experiment_name << " for participant " << it->first << std::endl;
            }
        }
    }

}
