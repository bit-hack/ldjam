#pragma once
#include <cstdint>
#include <memory>

#include "vec2.h"
#include "rect.h"

/* solid tiles denote which resolution vectors are available
 * non solid tiles denote which movement vectors are valid
 */

struct collision_map_t {

    bool collide(const rectf_t & r, vec2f_t & out);

    bool raycast(
            const vec2f_t & p0,
            const vec2f_t & p1,
            vec2f_t & hit);

    bool is_solid(vec2i_t & i) const {
        return index(i) & e_solid;
    }

protected:
    enum {
        e_solid         = 1,
        e_push_up       = 2,
        e_push_down     = 4,
        e_push_left     = 8,
        e_push_right    = 16
    };

    const vec2i_t size_;
    std::unique_ptr<uint8_t[]> map_;

    const uint8_t & index(const vec2i_t & i) const {
        assert(i.x >= 0 && i.x < size_.x);
        assert(i.y >= 0 && i.y < size_.y);
        return map_.get()[ i.x + i.y * size_.x ];
    }

    uint8_t & index(const vec2i_t & i) {
        assert(i.x >= 0 && i.x < size_.x);
        assert(i.y >= 0 && i.y < size_.y);
        return map_.get()[ i.x + i.y * size_.x ];
    }
};
