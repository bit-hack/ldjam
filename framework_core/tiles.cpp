#include "tiles.h"

bool collision_map_t::collide(const rectf_t &r, vec2f_t &out) {
    return false;
}

bool collision_map_t::collide(const vec2f_t &p, vec2f_t &out) {
    return false;
}

bool collision_map_t::raycast(const vec2f_t &p0, const vec2f_t &p1, vec2f_t &hit) {
    return false;
}

bool collision_map_t::preprocess() {
    return false;
}
