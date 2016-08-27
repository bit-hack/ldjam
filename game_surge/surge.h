#pragma once

#include <vector>

#include "../framework/objects.h"
#include "../framework/spatial.h"

#include "draw.h"

struct surge_t {

    draw_t & draw_;
    object_factory_t factory_;

    std::vector<object_ref_t> enemy_;
    object_ref_t player_;
    object_ref_t wave_;

    surge_t(draw_t & draw);

    void init();
    void tick();
};
