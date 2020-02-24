#include <array>
#include <cstdarg>
#include <cstdio>

#include "../framework_core/common.h"
#include "draw.h"

namespace tengu {
namespace {

template <typename typea_t, typename typeb_t>
float _orient2d(const vec2_t<typea_t>& a,
    const vec2_t<typea_t>& b,
    const vec2_t<typeb_t>& c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

} // namespace {}

void draw_t::triangle(
    const vec2f_t& v0,
    const vec2f_t& v1,
    const vec2f_t& v2)
{
    // triangle bounds
    int32_t minx = int32_t(minv(v0.x, v1.x, v2.x));
    int32_t maxx = int32_t(maxv(v0.x, v1.x, v2.x));
    int32_t miny = int32_t(minv(v0.y, v1.y, v2.y));
    int32_t maxy = int32_t(maxv(v0.y, v1.y, v2.y));
    // clip min point to screen
    minx = maxv<int32_t>(minx + 0, viewport_.x0);
    miny = maxv<int32_t>(miny + 0, viewport_.y0);
    maxx = minv<int32_t>(maxx + 1, viewport_.x1);
    maxy = minv<int32_t>(maxy + 1, viewport_.y1);
    // the signed triangle area
    const float area = 1.f / ((v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y));
    // reject back faces
    if (area <= 0.f)
        return;
    // barycentric x step
    const vec3f_t sx = {
        (v0.y - v1.y) * area, // A01
        (v1.y - v2.y) * area, // A12
        (v2.y - v0.y) * area // A20
    };
    // barycentric y step
    const vec3f_t sy = {
        (v1.x - v0.x) * area, // B01
        (v2.x - v1.x) * area, // B12
        (v0.x - v2.x) * area // B20
    };
    // barycentric start point
    vec2i_t p = { minx, miny };
    // barycentric value at start point
    vec3f_t wy = {
        _orient2d(v0, v1, p) * area, // W2 -> A01, B01
        _orient2d(v1, v2, p) * area, // W0 -> A12, B12
        _orient2d(v2, v0, p) * area, // W1 -> A20, B20
    };
    const uint32_t dst_pitch = target_->width();
    const uint32_t colour = colour_;
    uint32_t* pix = target_->data() + p.y * dst_pitch;
    // rendering loop
    for (p.y = miny; p.y < maxy; p.y++) {
        vec3f_t wx = wy;
        for (p.x = minx; p.x < maxx; p.x++) {
            // If p is on or inside all edges, render pixel.
            if (wx.x >= 0 && wx.y >= 0 && wx.z >= 0) {
                pix[p.x] = colour;
            }
            // X step
            wx += sx;
        }
        // Y step
        wy += sy;
        pix += dst_pitch;
    }
}
} // namespace tengu
