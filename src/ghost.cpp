#include <vr/ghost.h>
#include <vr/vr_service.h>

namespace vr {
    Ghost::Ghost(controller::Agent_operational_limits &limits, Vr_server &server) :
    limits(limits),
    server(server){};

    void Ghost::set_left(double new_left) {
        auto p_left = left;
        left = limits.convert(new_left);
        if (p_left != left) need_update = true;
    }

    void Ghost::set_right(double new_right) {
        auto p_right = right;
        right = limits.convert(new_right);
        if (p_right != right) need_update = true;
    }

    void Ghost::capture() {
        server.new_capture();
    }

    bool Ghost::update() {
        if (need_update) {
            server.set_ghost_movement((left + right) * forward_speed, (left - right) * rotation_speed);
            need_update = false;
        }
        return true;
    }

}