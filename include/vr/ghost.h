#pragma once
#include <controller.h>

namespace vr {
    struct Vr_server;

    struct Ghost : controller::Agent {

        Ghost(controller::Agent_operational_limits &limits, Vr_server &server);

        void set_left(double new_left) override;

        void set_right(double new_right) override;

        void capture() override;

        bool update() override;

        controller::Agent_operational_limits &limits;
        Vr_server &server;
        float left{};
        float right{};
        float rotation_speed{.5};
        float forward_speed{2};
        bool need_update = false;
    };
}