#pragma once
#include <controller.h>
#include <vr/vr_service.h>


namespace vr {
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
        bool need_update = false;
    };
}