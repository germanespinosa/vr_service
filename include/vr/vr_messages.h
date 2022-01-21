#pragma once
#include <json_cpp.h>

namespace vr {
    struct Vr_start_episode : json_cpp::Json_object{
        Json_object_members(
            Add_member(participant_id);
        );
        int participant_id;
    };
}