#include <array>
#include <cstdarg>
#include <cstdio>

#include "../framework_core/common.h"
#include "draw.h"

namespace tengu {
namespace {
const std::array<uint32_t, 5> _dither = {
    0x44001100u, // dither 1
    0x55005500u, // dither 2
    0x55aa55aau, // dither 3
    ~0x55005500u, // dither 4
    ~0x44001100u, // dither 5
};

void _draw_t_clip(const recti_t& viewport,
    recti_t& src,
    recti_t& dst)
{
    const recti_t& vp = viewport;
    // top left clip amount
    const int32_t lx = maxv(vp.x0 - dst.x0, 0);
    const int32_t ly = maxv(vp.y0 - dst.y0, 0);
    // adjust source and destination
    src.x0 += lx;
    src.y0 += ly;
    dst.x0 += lx;
    dst.y0 += ly;
    // clip lower right
    dst.x1 = minv(vp.x1, dst.x1);
    dst.y1 = minv(vp.y1, dst.y1);
}

// pixel blending function type
typedef uint32_t pixel_func_t(const uint32_t src,
    const uint32_t dest,
    const uint32_t colour,
    const uint32_t key,
    const uint32_t order);

// opaque blend mode (100% src, no key)
constexpr uint32_t _blend_opaque(const uint32_t src,
    const uint32_t,
    const uint32_t,
    const uint32_t,
    const uint32_t)
{
    return src;
}

// colour key blend mode
constexpr uint32_t _blend_key(const uint32_t src,
    const uint32_t dst,
    const uint32_t,
    const uint32_t key,
    const uint32_t)
{
    return (src == key) ? dst : src;
}

// 50% transparency
constexpr uint32_t _blend_gliss(const uint32_t src,
    const uint32_t dst,
    const uint32_t,
    const uint32_t key,
    const uint32_t)
{
    return (src == key) ? (dst) : ((src >> 1) & 0x7f7f7f) + ((dst >> 1) & 0x7f7f7f);
}

// colour keyed overlay
constexpr uint32_t _blend_mask(const uint32_t src,
    const uint32_t dst,
    const uint32_t colour,
    const uint32_t key,
    const uint32_t)
{
    return (src == key) ? (dst) : (colour);
}

// ordered dither
template <int32_t _ORDER>
constexpr uint32_t _blend_dither(const uint32_t src,
    const uint32_t dst,
    const uint32_t colour,
    const uint32_t key,
    const uint32_t order)
{
    // ordered dither
    return (src == key || (_dither[_ORDER] & (1 << order)) == 0) ? (dst) : (src);
}

} // namespace {}

void draw_t::clear()
{
    assert(target_);
    const uint32_t pitch = target_->width();
    const uint32_t colour = colour_;
    recti_t viewport = viewport_;
    uint32_t* pix = target_->data() + viewport.y0 * pitch;
    for (int y = viewport.y0; y <= viewport.y1; ++y) {
        _span(viewport.x0, viewport.x1, y);
        pix += pitch;
    }
}

void draw_t::circle(
    const vec2i_t& center,
    const int32_t radius)
{
    assert(target_);

    int32_t xC = center.x, yC = center.y;
    int32_t p = 1 - radius, x = 0, y = radius;
    _span(xC - y, xC + y, yC);
    while (x++ <= y) {
        if (p < 0) {
            p += 2 * x + 1;
        } else {
            p += 2 * (x - y) + 1;
            y--;
        }
        _span(xC - x, xC + x, yC + y);
        _span(xC - x, xC + x, yC - y);
        _span(xC - y, xC + y, yC + x);
        _span(xC - y, xC + y, yC - x);
    }
}

void draw_t::rect(const recti_t p)
{
    assert(target_);
    const recti_t rect = recti_t::intersect(viewport_ + recti_t{ 0, 0, 1, 1 }, p);
    const uint32_t pitch = target_->width();
    const uint32_t colour = colour_;
    uint32_t* pix = target_->data() + rect.y0 * pitch;
    for (int y = rect.y0; y < rect.y1; ++y) {
        for (int x = rect.x0; x < rect.x1; ++x) {
            pix[x] = colour;
        }
        pix += pitch;
    }
}

void draw_t::line(
    const vec2f_t& p0,
    const vec2f_t& p1)
{
    line(vec2i_t{ int32_t(p0.x), int32_t(p0.y) },
        vec2i_t{ int32_t(p1.x), int32_t(p1.y) });
}

void draw_t::line(
    const vec2i_t& p0,
    const vec2i_t& p1)
{
    // <---- ---- ---- ---- ---- ---- ---- ---- todo: clip line to viewport
    bool yLonger = false;
    int32_t x = p0.x, y = p0.y;
    int32_t incrementVal, endVal;
    int32_t shortLen = p1.y - p0.y, longLen = p1.x - p0.x;
    if (abs(shortLen) > abs(longLen)) {
        swapv(shortLen, longLen);
        yLonger = true;
    }
    endVal = longLen;
    if (longLen < 0) {
        incrementVal = -1;
        longLen = -longLen;
    } else {
        incrementVal = 1;
    }
    const int32_t decInc = (longLen == 0) ? 0 : (shortLen << 16) / longLen;
    if (yLonger) {
        for (int32_t i = 0, j = 0; i != endVal; i += incrementVal, j += decInc) {
            plot(vec2i_t{ x + (j >> 16), y + i });
        }
    } else {
        for (int32_t i = 0, j = 0; i != endVal; i += incrementVal, j += decInc) {
            plot(vec2i_t{ x + i, y + (j >> 16) });
        }
    }
}

void draw_t::plot(const vec2i_t& p)
{
    assert(target_);
    if (viewport_.contains(p)) {
        const uint32_t colour = colour_;
        target_->data()[p.x + p.y * target_->width()] = colour;
    }
}

void draw_t::set_target(struct bitmap_t& bmp)
{
    target_ = &bmp;
    viewport_ = _target_size();
}

void draw_t::viewport(const recti_t& rect)
{
    recti_t vp = _target_size();
    viewport_ = recti_t::intersect(vp, rect);
}

void draw_t::viewport()
{
    viewport_ = _target_size();
}

void draw_t::copy(
    struct bitmap_t& dst,
    const recti_t& src_rect,
    const vec2i_t& dst_pos)
{
}

void draw_t::_span(int32_t x0, int32_t x1, int32_t y)
{
    if (y >= viewport_.y0 && y <= viewport_.y1) {
        x0 = maxv(x0, viewport_.x0);
        x1 = minv(x1, viewport_.x1);
        uint32_t* pix = target_->data();
        pix += y * target_->width();
        const uint32_t colour = colour_;
        for (int32_t x = x0; x <= x1; ++x) {
            pix[x] = colour;
        }
    }
}

recti_t draw_t::_target_size() const
{
    assert(target_);
    return recti_t{
        0,
        0,
        target_->width() - 1,
        target_->height() - 1
    };
}

} // namespace tengu
