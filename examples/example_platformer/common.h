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

#include "input.h"


enum {
    e_object_map = 0,
    e_object_player_shadow,
    e_object_particles,
    e_object_player_splash,
    e_object_player,
    e_object_camera,
};

struct service_t {
    tengu::draw_ex_t draw_;
    tengu::object_factory_t & factory_;
    gamepad_t * gamepad_;
    tengu::object_map_t objects_;
};
