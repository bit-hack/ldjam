#pragma once
#include <cstdint>
#include <memory>

#include "vec2.h"
#include "rect.h"

/* solid tiles denote which resolution vectors are available
 * non solid tiles denote which movement vectors are valid
 */

enum {
    e_tile_solid         = 1,
    e_tile_push_up       = 2,
    e_tile_push_down     = 4,
    e_tile_push_left     = 8,
    e_tile_push_right    = 16
};

struct collision_map_t {

    collision_map_t(const vec2i_t & size, const vec2i_t & cell_size)
        : size_(size)
        , cell_size_(cell_size)
        , map_(new uint8_t[ size.x * size.y ])
    {
    }

    // collide a bounding rect with solid tiles in the collision map.
    // this is the most robust solver of the two.  the need for solid
    // tiles however may be a limitation in some situations.
    bool collide(const rectf_t & r, vec2f_t & out);

    // collide a bounding rect with the collision map via tile flags.
    // In this case we do not require solid tiles, only edge flags in
    // all the tiles.  this enables us to have zero tile thick walls.
    // this solver is less robust then the former however, and doesnt
    // handle multiple tile collisions as well.
    bool collide_alt(const rectf_t & r, vec2f_t & out);

    // collide a point with the map tiles.
    bool collide(const vec2f_t & p, vec2f_t & out);

    bool raycast(
            const vec2f_t & p0,
            const vec2f_t & p1,
            vec2f_t & hit);

    bool preprocess();

    bool is_solid(const vec2i_t & p) const {
        assert(map_.get());
        if (p.x >= 0 && p.x < size_.x && p.y >= 0 && p.y < size_.y) {
            return (get(p) & e_tile_solid) != 0;
        }
        else {
            // off the map is unconditional solid
            return true;
        }
    }

    uint8_t * get() const {
        assert(map_.get());
        return map_.get();
    }

    vec2i_t size() const {
        return size_;
    }

    uint8_t & get(const vec2i_t & p) {
        assert(p.x >= 0 && p.x < size_.x);
        assert(p.y >= 0 && p.y < size_.y);
        assert(map_.get());
        return map_.get()[p.x + p.y * size_.x];
    }

    const uint8_t & get(const vec2i_t & p) const {
        assert(p.x >= 0 && p.x < size_.x);
        assert(p.y >= 0 && p.y < size_.y);
        assert(map_.get());
        return map_.get()[p.x + p.y * size_.x];
    }

    void clear(const uint8_t value) {
        assert(map_.get());
        const uint32_t count = size_.x * size_.y;
        uint8_t * tile = map_.get();
        for (int32_t i=0; i<count; ++i) {
            *(tile++) = value;
        }
    }

    vec2i_t cell_size() const {
        return cell_size_;
    }

protected:

    const vec2i_t size_;
    const vec2i_t cell_size_;
    std::unique_ptr<uint8_t[]> map_;
};
