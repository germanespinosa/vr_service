#include <vr/vr_agent_state.h>

namespace vr {

    bool Agent_state::operator==(const Agent_state &other) const{
        return other.location == location && other.rotation == rotation;
    }

    Agent_state::Agent_state(const std::string &agent_name, float x,float y,float z,float roll,float pitch, float yaw):
        agent_name(agent_name) {
        location.x = x;
        location.y = x;
        location.z = x;
        rotation.roll = roll;
        rotation.pitch = pitch;
        rotation.yaw = yaw;
    }

    bool Rotation3::operator==(const Rotation3 &other) const{
        return other.roll == roll && other.pitch == pitch && other.yaw == yaw;
    }

    bool Location3::operator==(const Location3 &other) const{
        return other.x == x && other.y == y && other.z == z;
    }

    cell_world::Location Location3::to_location() const {
        return {x, y};
    }

}