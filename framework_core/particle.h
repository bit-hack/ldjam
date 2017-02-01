#pragma once

#include "vec2.h"
#include <memory>

struct particles_t {

    //todo: remove hard coded particle limit

    particles_t(const vec2f_t pos, const size_t count)
        : count_(count)
        , centre_(pos)
    {
        part_.age_.reset(new float[count_]);
        part_.p1_.reset(new vec2f_t[count_]);
        part_.p0_.reset(new vec2f_t[count_]);
    }

    virtual void particle_tick() = 0;

    //todo: add spawn, add attractors, add repelant, add flow field

protected:
    size_t count_;
    vec2f_t centre_;

    struct {
        std::unique_ptr<float[]> age_;
        std::unique_ptr<vec2f_t[]> p1_;
        std::unique_ptr<vec2f_t[]> p0_;
    } part_;
};
