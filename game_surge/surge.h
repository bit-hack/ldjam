#pragma once

#include <vector>

#include "../framework/objects.h"
#include "../framework/spatial.h"

#include "draw.h"

struct surge_t {

    draw_t & draw_;
    object_factory_t factory_;
    spatial_t spatial_;

    uint32_t difficulty_;

    std::vector<object_ref_t> enemy_;
    object_ref_t player_;
    object_ref_t wave_;
    object_ref_t stars_;

    surge_t(draw_t & draw);

    void init();
    void tick();
};
