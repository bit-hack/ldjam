#pragma once

#include <SDL/SDL.h>

#include "../../framework_core/fsm.h"
#include "../../framework_core/random.h"
#include "../../framework_core/objects.h"
#include "../../framework_core/timer.h"
#include "../../framework_core/anim.h"
#include "../../framework_draw/draw.h"
#include "../../framework_tiles/tiles.h"

enum {
    e_object_player,
    e_object_camera
};

struct service_t {
    object_factory_t * factory_;
    draw_t * draw_;
    collision_map_t * map_;
    object_ref_t player_;
};
