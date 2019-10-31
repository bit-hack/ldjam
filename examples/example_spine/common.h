#pragma once

#include <SDL/SDL.h>
#include <unordered_map>
#include <string>

#include "../../framework_core/fsm.h"
#include "../../framework_core/random.h"
#include "../../framework_core/objects.h"
#include "../../framework_core/timer.h"
#include "../../framework_core/anim.h"
#include "../../framework_draw/draw.h"
#include "../../framework_tiles/tiles.h"

enum {
    e_object_car = 0,
};

struct service_t {
    draw_ex_t draw_;
    object_factory_t & factory_;
    object_map_t objects_;
};
