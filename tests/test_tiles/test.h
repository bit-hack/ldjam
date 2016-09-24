#pragma once

#include "../../framework_draw/draw.h"

struct test_t {
    virtual void tick() = 0;
};

extern test_t * new_test_astar(draw_t & draw);
