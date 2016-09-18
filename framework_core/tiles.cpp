#include "../framework_core/common.h"
#include "tiles.h"

namespace {
    template <typename type_t>
    constexpr type_t select_abs_min(const type_t & a, const type_t & b) {
        return (absv(a) < absv(b)) ? a : b;
    }
}

bool collision_map_t::collide(const rectf_t &r, vec2f_t &out) {

    int32_t minx = clampv(0, quantize<int32_t>(r.x0, cell_size_.x), size_.x-1);
    int32_t miny = clampv(0, quantize<int32_t>(r.y0, cell_size_.y), size_.y-1);
    int32_t maxx = clampv(0, quantize<int32_t>(r.x1, cell_size_.x), size_.x-1);
    int32_t maxy = clampv(0, quantize<int32_t>(r.y1, cell_size_.y), size_.y-1);

    bool collide = false;

    const float ival = max2(r.x1-r.x0, r.y1-r.y0);
    out.x = ival;
    out.y = ival;

    bool setx = false, sety = false;

    for (int32_t y=miny; y<=maxy; ++y) {
        for (int32_t x=minx; x<=maxx; ++x) {

            const uint8_t t = get(vec2i_t{x, y});

            if ((t & e_tile_solid) == 0)
                continue;

            // we have now collided for sure
            collide = true;

            const rectf_t
                    b { float(x+0) * cell_size_.x,
                        float(y+0) * cell_size_.y,
                        float(x+1) * cell_size_.x,
                        float(y+1) * cell_size_.y };

            // find all possible resolution vectors (r = collider, b = blocker)
            std::array<float, 4> res = {
                (t & e_tile_push_up)    ? b.y0 - r.y1 : -ival,
                (t & e_tile_push_down)  ? b.y1 - r.y0 :  ival,
                (t & e_tile_push_left)  ? b.x0 - r.x1 : -ival,
                (t & e_tile_push_right) ? b.x1 - r.x0 :  ival,
            };

            res[0] = select_abs_min(res[0], res[1]);
            res[2] = select_abs_min(res[2], res[3]);

            if (absv(res[0]) < absv(res[2])) {
                out.y = (absv(res[0]) < absv(out.y)) ? res[0] : out.y;
                sety = true;
            }
            else {
                out.x = (absv(res[2]) < absv(out.x)) ? res[2] : out.x;
                setx = true;
            }
        }
    }

    out.x = setx ? out.x : 0.f;
    out.y = sety ? out.y : 0.f;

    return collide;
}

bool collision_map_t::collide(const vec2f_t &p, vec2f_t &out) {
    return false;
}

bool collision_map_t::raycast(const vec2f_t &p0, const vec2f_t &p1, vec2f_t &hit) {
    return false;
}

bool collision_map_t::preprocess() {
    for (int32_t y=0; y<size_.y; ++y) {
        for (int32_t x=0; x<size_.x; ++x) {
            uint8_t & flags = get(vec2i_t{x, y});
            flags |= is_solid(vec2i_t{x-1, y}) ? 0 : e_tile_push_left;
            flags |= is_solid(vec2i_t{x+1, y}) ? 0 : e_tile_push_right;
            flags |= is_solid(vec2i_t{x, y-1}) ? 0 : e_tile_push_up;
            flags |= is_solid(vec2i_t{x, y+1}) ? 0 : e_tile_push_down;
        }
    }
    return true;
}
