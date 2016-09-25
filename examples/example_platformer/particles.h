#pragma once
#include <array>

#include "common.h"
#include "../../framework_core/common.h"
#include "../../framework_core/vec2.h"

struct dust_t : public object_ex_t<e_object_particles, dust_t> {

    static const uint32_t _ORDER = 2;

    dust_t(object_service_t service)
        : service_(*static_cast<service_t*>(service))
        , rand_(SDL_GetTicks())
    {
        order_ = _ORDER;
    }

    void init(
        const uint32_t count,
        const vec2f_t & center,
        const vec2f_t & dir,
        const vec2f_t & grav,
        const float spread)
    {
        count_ = count;
        grav_ = grav;

        age_.reset(new float[count]);
        p0_.reset(new vec2f_t[count]);
        p1_.reset(new vec2f_t[count]);

        float * age = age_.get();
        vec2f_t * p0 = p0_.get();
        vec2f_t * p1 = p1_.get();
        for (uint32_t i=0; i<count_; ++i) {

            age[i] = 8.f;

            p1[i] = center;
            vec2f_t pos;
            rand_.randfs_vec2(pos);
            p0[i] = pos * spread + center + dir;
        }
    }

    virtual void tick() override {

        alive_ = false;
        float * age = age_.get();
        vec2f_t * p0 = p0_.get();
        vec2f_t * p1 = p1_.get();

        service_.draw_.colour_ = 0xD0F0D0;

        for (uint32_t i=0; i<count_; ++i) {
            const vec2f_t vel = p1[i] - p0[i];

            p0[i] = p1[i];
            p1[i] += vel + grav_;
            age[i] -= 1.f;

            const float size = clampv(1.f, age[i], 3.f);
            service_.draw_.circle<true>(vec2i_t(p1[i]), int32_t(size));

            alive_ |= age[i] > 0.f;
        }

        if (!alive_) {
            ref_.dec();
        }
    }

    random_t rand_;
    vec2f_t grav_;
    uint32_t count_;
    service_t & service_;
    std::unique_ptr<float[]> age_;
    std::unique_ptr<vec2f_t[]> p0_;
    std::unique_ptr<vec2f_t[]> p1_;
};
