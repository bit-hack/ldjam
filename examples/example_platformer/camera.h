#pragma once
#include "common.h"

struct camera_t : public object_ex_t<e_object_camera, camera_t> {

    // note: should update after all visible entities
    static const uint32_t _ORDER = 6;

    service_t & service_;

    vec2f_t target_;
    vec2f_t pos_;

    camera_t(object_service_t s);

    void shake();

    virtual void tick() override;
};
