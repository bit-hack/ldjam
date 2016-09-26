#pragma once
#include "common.h"
#include "../../framework_core/timer.h"

struct camera_t : public object_ex_t<e_object_camera, camera_t> {

    // note: should update after all visible entities
    static const uint32_t _ORDER = 6;

    service_t & service_;
    vec2f_t target_;
    vec2f_t pos_;

    struct shake_t {

        shake_t()
            : rand_(0x1234)
            , mag_(0.f)
            , counter_(SDL_GetTicks, 8)
        {
            offset_[0] = vec2f_t{0, 0};
            offset_[1] = vec2f_t{0, 0};
        }

        random_t rand_;
        float mag_;
        timing_t<uint32_t> counter_;
        std::array<vec2f_t, 2> offset_;
    } shake_;

    camera_t(object_service_t s);

    void shake(float mag);

    virtual void tick() override;
};
