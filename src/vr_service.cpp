#include <vr/vr_service.h>
#include <experiment/experiment_client.h>
#include <agent_tracking/tracking_service.h>
#include <easy_tcp.h>
#include <controller/agent.h>
#include <map>

using namespace std;
using namespace agent_tracking;
using namespace experiment;
using namespace cell_world;
using namespace controller;

namespace vr {

    struct Active_experiment_data {
        string experiment_name;
        Cell_group_builder occlusions;
        Location_list ghost_spawn_locations;
        bool is_active;
    };

    Tracking_server vr_tracking;
    Vr_server *vr_server = nullptr;
    map<int, Active_experiment_data> active_experiments;
    Active_experiment_data last_experiment_started;
    bool pending_participant = false;
    float forward_speed = .5;
    float rotation_speed = .5;
    float ghost_min_distance = 0;
    Location prey_start_location;
    World_configuration configuration;
    World_implementation implementation;
    Space tracking_space;
    World world;
    Scale vr_scale(-1,1);


    struct : experiment::Experiment_client {
        void on_experiment_started(const Start_experiment_response  &experiment) override {
            last_experiment_started.experiment_name = experiment.experiment_name;
            last_experiment_started.occlusions = Resources::from("cell_group").key(experiment.world.world_configuration).key(experiment.world.occlusions).key("occlusions").get_resource<Cell_group_builder>();
            last_experiment_started.is_active = true;
            world.set_occlusions(last_experiment_started.occlusions);
            auto free_cells = world.create_cell_group().free_cells();
            last_experiment_started.ghost_spawn_locations.clear();
            for (auto &cell:free_cells){
                auto location = cell.get().location;
                if (prey_start_location.dist(location)>ghost_min_distance)
                    last_experiment_started.ghost_spawn_locations.push_back(location);
            }
            pending_participant = true;
            cout << "Experiment " << experiment.experiment_name << " started, waiting for participant" << endl;
        };

        void on_experiment_finished(const std::string &experiment_name) override {
            map<int, Active_experiment_data>::iterator it;
            for(it=active_experiments.begin(); it!=active_experiments.end(); ++it){
                if (it->second.experiment_name == experiment_name) {
                    active_experiments.erase(it->first );
                    cout << "Closing experiment " << experiment_name << " for participant " << it->first << endl;
                }
            }
        };
    } experiment_client;

    void Vr_service::set_agent_state(const Agent_state &agent_state) {
        cell_world::Step step;
        step.agent_name = agent_state.agent_name;
        step.frame = agent_state.frame;
        step.location = implementation.space.scale(agent_state.location.to_location(), Scale{-1,1});
        step.rotation = agent_state.rotation.yaw - 90;
        step.time_stamp = agent_state.time_stamp;
        auto converted = step.convert(implementation.space, tracking_space);
        vr_tracking.send_step(converted);
    }

    Vr_start_episode_response Vr_service::start_episode(const Vr_start_episode_request &parameters) {
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

    int Vr_service::get_port() {
        string port_str(std::getenv("CELLWORLD_VR_SERVICE_PORT") ? std::getenv("CELLWORLD_VR_SERVICE_PORT") : "4580");
        return atoi(port_str.c_str());
    }

    void Vr_service::start_tracking_service(int port) {
        vr_tracking.start(port);
    }

    Vr_finish_episode_response Vr_service::finish_episode(const Vr_finish_episode_request &parameters) {
        Vr_finish_episode_response response;
        if (active_experiments.contains(parameters.participant_id)){
            thread async_finish_episode ( [] (Vr_finish_episode_request parameters) {
                auto &experiment = active_experiments[parameters.participant_id];
                experiment_client.finish_episode();
                cout << "Starting episode for experiment " << experiment.experiment_name;
                experiment.is_active = experiment_client.is_active(experiment.experiment_name);
            }, parameters);
            async_finish_episode.detach();
        }
        return response;
    }

    World_implementation Vr_service::get_world_implementation() {
        return implementation;
    }

    bool Vr_service::connect_experiment_service(const string &ip) {
        if (experiment_client.connect(ip)) {
            experiment_client.set_tracking_service_ip("127.0.0.1");
            experiment_client.subscribe();
            return true;
        }
        return false;
    }

    void Vr_service::set_world(const cell_world::World_configuration &new_configuration, const cell_world::World_implementation &new_implementation) {
        configuration = new_configuration;
        implementation = new_implementation;
        implementation.scale(vr_scale);
        world = World(configuration, implementation);
    }

    void Vr_service::set_ghost_min_distance(float new_min_distance) {
        ghost_min_distance = new_min_distance;
    }

    void Vr_service::set_prey_start_location(const cell_world::Location &new_location) {
        prey_start_location = new_location;
    }

    void Vr_service::set_tracking_space(const Space &new_tracking_space) {
        tracking_space = new_tracking_space;
    }

    void Vr_server::set_ghost_movement(float forward, float rotation) {
        Vr_update_ghost_movement parameters;
        parameters.forward = forward;
        parameters.rotation = rotation;
        if (vr_server) {
            vr_server->broadcast_subscribed(tcp_messages::Message("update_ghost_movement",parameters));
            cout << parameters << endl;
        }
    }

    Vr_server::Vr_server() {
        vr_server = this;
    }

    void Vr_server::on_new_connection(Vr_service &new_connection) {
        cout << "New connection" << endl;
        Message_server::on_new_connection(new_connection);
    }

    void Vr_server::capture() {
        if (vr_server) {
            vr_server->broadcast_subscribed(tcp_messages::Message("capture"));
        }
    }
}
