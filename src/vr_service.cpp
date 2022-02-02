#include <vr/vr_service.h>
#include <experiment/experiment_client.h>
#include <agent_tracking/tracking_service.h>
#include <easy_tcp.h>
#include <map>

using namespace std;
using namespace agent_tracking;
using namespace experiment;
using namespace cell_world;

namespace vr {
    Tracking_server vr_tracking;
    Vr_server *vr_server = nullptr;
    map<int, Start_experiment_response> active_experiments;
    Start_experiment_response last_experiment_started;
    bool pending_participant = false;
    float forward_speed = 1.0;
    float rotation_speed = 1.0;
    World_configuration configuration;
    World_implementation implementation;

    struct Ghost : easy_tcp::Service {
        void on_incoming_data(const char *buff, int size) override{
            if (size == 3){ // instruction
                float left = buff[0];
                float right = buff[1];
                Vr_server::set_ghost_movement((right + left) * forward_speed, (right - left) * rotation_speed);
            }
        }
    };
    easy_tcp::Server<Ghost> ghost;


    struct : experiment::Experiment_client {
        void on_experiment_started(const Start_experiment_response &experiment) override {
            last_experiment_started = experiment;
            pending_participant = true;
            cout << "Experiment " << experiment.experiment_name << " started, waiting for participant" << endl;
        };

        void on_experiment_finished(const std::string &experiment_name) override {
            map<int, Start_experiment_response>::iterator it;
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
        step.location = agent_state.location.to_location();
        step.rotation = agent_state.rotation.yaw;
        step.time_stamp = agent_state.time_stamp;
        agent_tracking::Tracking_service::send_step(step);
        cout << agent_state << endl;
    }

    Vr_start_episode_response Vr_service::start_episode(const Vr_start_episode_request &parameters) {
        Vr_start_episode_response response;
        Start_experiment_response experiment;
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
        cout << "Starting episode for experiment " << experiment.experiment_name;
        experiment_client.start_episode(experiment.experiment_name);
        response.occlusions = Resources::from("cell_group").key(experiment.world.world_configuration).key(experiment.world.occlusions).key("occlusions").get_resource<Cell_group_builder>();
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
            auto experiment = active_experiments[parameters.participant_id];
            experiment_client.finish_episode();
            cout << "Starting episode for experiment " << experiment.experiment_name;
            response.is_active = experiment_client.is_active(experiment.experiment_name);
        }
        return response;
    }

    World_implementation Vr_service::get_world_implementation() {
        return implementation;
    }

    bool Vr_service::connect_experiment_service(const string &ip) {
        if (experiment_client.connect(ip)) {
            experiment_client.set_tracking_service_ip("127.0.0.1");
            return true;
        }
        return false;
    }

    void Vr_service::set_ghost_forward_speed(float new_forward_speed) {
        forward_speed = new_forward_speed;
    }

    void Vr_service::set_ghost_rotation_speed(float new_rotation_speed) {
        rotation_speed = new_rotation_speed;
    }

    void Vr_service::set_configuration(const string &configuration_name) {
        configuration = Resources::from("world_configuration").key(configuration_name).get_resource<World_configuration>();
    }

    void Vr_service::set_implementation(const string &implementation_name) {
        implementation = Resources::from("world_implementation").key(implementation_name).get_resource<World_implementation>();
    }

    void Vr_server::set_ghost_movement(float forward, float rotation) {
        Vr_update_ghost_movement parameters;
        parameters.forward = forward;
        parameters.rotation = rotation;
        if (vr_server) {
            vr_server->broadcast_subscribed(tcp_messages::Message("update_ghost_movement",parameters));
        }
    }

    Vr_server::Vr_server() {
        vr_server = this;
    }
}
