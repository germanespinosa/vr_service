#pragma once
#include <json_cpp.h>
#include <cell_world.h>
namespace vr {
    struct Vr_get_cell_locations_response : json_cpp::Json_object{
        Json_object_members(
                Add_member(cell_locations);
        );
        cell_world::Location_list cell_locations;
    };
    struct Vr_start_episode_request : json_cpp::Json_object{
        Json_object_members(
            Add_member(participant_id);
        );
        int participant_id;
    };
    struct Vr_start_episode_response : json_cpp::Json_object{
        Json_object_members(
                Add_member(occlusions);
                Add_member(predator_spawn_location);
        );
        cell_world::Cell_group_builder occlusions;
        cell_world::Location predator_spawn_location;
    };
    struct Vr_finish_episode_request : json_cpp::Json_object{
        Json_object_members(
                Add_member(participant_id);
        );
        int participant_id;
    };
    struct Vr_finish_episode_response : json_cpp::Json_object{
        Json_object_members(
                Add_member(is_active);
        );
        bool is_active;
    };

    struct Vr_update_ghost_movement : json_cpp::Json_object{
        Json_object_members(
                Add_member(forward);
                Add_member(rotation);
        );
        float forward;
        float rotation;
    };

}