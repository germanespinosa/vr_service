#pragma once
#include <json_cpp.h>
#include <cell_world.h>

namespace vr {

    struct Location3 : json_cpp::Json_object{
        Json_object_members(
                Add_member(x);
                Add_member(y);
                Add_member(z);
                );
        float x;
        float y;
        float z;
        bool operator == (const Location3 &) const;
        cell_world::Location to_location() const;
    };

    struct Rotation3 : json_cpp::Json_object{
        Json_object_members(
                Add_member(roll);
                Add_member(pitch);
                Add_member(yaw);
                );
        float roll;
        float pitch;
        float yaw;
        bool operator == (const Rotation3 &) const;
    };

    struct Agent_state : json_cpp::Json_object {
        Agent_state() = default;
        Agent_state(const std::string &agent_name, float x,float y,float z,float roll,float pitch, float yaw);
        Json_object_members(
                Add_member(time_stamp);
                Add_member(frame);
                Add_member(agent_name);
                Add_member(location);
                Add_member(rotation);
                );
        float time_stamp;
        unsigned int frame;
        Location3 location;
        Rotation3 rotation;
        std::string agent_name;
        bool operator == (const Agent_state &) const;
    };
}