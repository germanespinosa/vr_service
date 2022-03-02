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

    void Vr_server::new_capture() {
        broadcast_subscribed(Message("capture"));
    }

    cell_world::World_implementation Vr_server::get_world_implementation() const {
        return vr_implementation;
    }

    Vr_start_episode_response Vr_server::start_episode(const Vr_start_episode_request &parameters) {
        Vr_start_episode_response response;
        Active_experiment_data experiment;
        if (pending_participant) { // first episode of the new experiment
            experiment = last_experiment_started;
            cout << "Experiment " << experiment.experiment_name << " associated to participant " << parameters.participant_id << endl;
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
        episode_in_progress = true;
        cout << response << endl;
        return response;
    }

    Vr_finish_episode_response Vr_server::finish_episode(const Vr_finish_episode_request &parameters) {
        Vr_finish_episode_response response;
        if (active_experiments.contains(parameters.participant_id)){
            auto &experiment = active_experiments[parameters.participant_id];
            experiment_client.finish_episode();
            experiment.is_active = experiment_client.is_active(experiment.experiment_name);
            episode_in_progress = false;
        }
        return response;
    }

    void Vr_server::set_agent_state(const Agent_state &agent_state) {
        cell_world::Step step;
        step.agent_name = agent_state.agent_name;
        step.frame = agent_state.frame;
        step.location = vr_implementation.space.scale(agent_state.location.to_location(), Scale{-1,1});
        step.rotation = agent_state.rotation.yaw - 90;
        step.time_stamp = agent_state.time_stamp;
        auto converted = step.convert(vr_implementation.space, canonical_implementation.space);
        tracking_server.send_step(converted);
    }

    Vr_server::Vr_server(
            const cell_world::World_configuration &configuration,
            const cell_world::World_implementation &vr_implementation,
            const cell_world::World_implementation &canonical_implementation) :
            experiment_server(),
            tracking_server(),
            configuration(configuration),
            vr_implementation(vr_implementation),
            vr_world(configuration, vr_implementation),
            canonical_implementation(canonical_implementation),
            canonical_world(configuration, canonical_implementation),
            canonical_visibility(canonical_world.create_location_visibility()),
            capture(Resources::from("capture_parameters").key("default").get_resource<Capture_parameters>(), canonical_world),
            peeking(Resources::from("peeking_parameters").key("default").get_resource<Peeking_parameters>(), canonical_world),
            experiment_tracking_client(tracking_server.create_local_client<Experiment_tracking_client>()),
            controller_tracking_client(tracking_server.create_local_client<Controller_server::Controller_tracking_client>(
                    canonical_world,
                    float(90),
                    "predator",
                    "prey")),
            ghost_operational_limits(json_cpp::Json_from_file<Agent_operational_limits>("../config/ghost_operational_limits.json")),
            ghost(ghost_operational_limits, *this ),
            controller_server(
                "../config/pid.json",
                ghost,
                controller_tracking_client,
                experiment_server.create_local_client<Controller_server::Controller_experiment_client>()),
                experiment_client(experiment_server.create_local_client<Vr_experiment_client>()
            ){
        pending_participant = false;
        experiment_client.vr_server = this;
        experiment_tracking_client.experiment_server = &experiment_server;
        experiment_tracking_client.subscribe();
        if (!experiment_server.start(Experiment_service::get_port())){
            cout << "Failed to start experiment service" << endl;
            exit(1);
        }
        experiment_server.set_tracking_client(experiment_tracking_client);
        experiment_client.subscribe();
    }

    void Vr_server::set_occlusions(const Cell_group_builder &occlusions) {
        canonical_world.set_occlusions(occlusions);
        vr_world.set_occlusions(occlusions);
        auto cells = canonical_world.create_cell_group();
        capture.visibility.update_occlusions(cells);
        peeking.peeking_visibility.update_occlusions(cells);
        controller_server.navigability.update_occlusions(cells);
        controller_tracking_client.set_occlusions(cells);
    }

    void Vr_server::on_experiment_started(const Start_experiment_response &experiment) {
        last_experiment_started.experiment_name = experiment.experiment_name;
        last_experiment_started.is_active = true;
        last_experiment_started.occlusions = Cell_group_builder::get_from_parameters_name("hexagonal", experiment.world.occlusions + ".occlusions" );
        auto free_cells = canonical_world.create_cell_group().free_cells();
        last_experiment_started.ghost_spawn_locations.clear();
        for (auto &cell:free_cells){
            auto location = cell.get().location;

            if (prey_start_location.dist(location)>ghost_min_distance) {
                auto vr_location = vr_implementation.cell_locations[cell.get().id];
                last_experiment_started.ghost_spawn_locations.push_back(vr_location);
            }
        }
        pending_participant = true;
        std::cout << "Experiment " << experiment.experiment_name << " started, waiting for participant" << std::endl;
    }

    void Vr_server::chasing() {
        cout << "Ghost behavior set to chasing" << endl;
        broadcast_subscribed(Message("chasing"));
    }

    void Vr_server::exploring() {
        cout << "Ghost behavior set to exploring" << endl;
        broadcast_subscribed(Message("exploring"));
    }

    void Vr_server::Vr_experiment_client::on_experiment_started(const Start_experiment_response &experiment) {
        vr_server->on_experiment_started(experiment);
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

    void Vr_server::Vr_experiment_client::on_behavior_set(int behavior) {
        if (behavior == 1) {
            vr_server->chasing();
        }else {
            vr_server->exploring();
        }
    }
}
