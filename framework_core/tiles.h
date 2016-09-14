#pragma once
#include <cstdint>
#include <memory>

#include "vec2.h"
#include "rect.h"

/* solid tiles denote which resolution vectors are available
 * non solid tiles denote which movement vectors are valid
 */

struct collision_map_t {

    collision_map_t(const vec2i_t & size)
        : size_(size)
        , map_(new uint8_t[ size.x * size.y ])
    {
    }

    bool collide(const rectf_t & r, vec2f_t & out);

    bool collide(const vec2f_t & p, vec2f_t & out);

    bool raycast(
            const vec2f_t & p0,
            const vec2f_t & p1,
            vec2f_t & hit);

    bool preprocess();

    bool is_solid(vec2i_t & i) const {
        return index(i) & e_solid;
    }

    uint8_t * get() const {
        assert(map_.get());
        return map_.get();
    }

    vec2i_t size() const {
        return size_;
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
        assert(map_.get());
        return map_.get()[ i.x + i.y * size_.x ];
    }

    uint8_t & index(const vec2i_t & i) {
        assert(i.x >= 0 && i.x < size_.x);
        assert(i.y >= 0 && i.y < size_.y);
        assert(map_.get());
        return map_.get()[ i.x + i.y * size_.x ];
    }
};
