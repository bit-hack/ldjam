#include <array>
#include <cstdarg>
#include <cstdio>

#include "../framework_core/common.h"
#include "draw.h"

namespace {

static inline uint32_t alpha_blend(
    const uint32_t a,
    const uint32_t b,
    const uint8_t i)
{
#if 0
  // unpack a
  const uint8_t a_r = (a >> 16);
  const uint8_t a_g = (a >>  8);
  const uint8_t a_b = (a >>  0);
  // unpack b
  const uint8_t b_r = (b >> 16);
  const uint8_t b_g = (b >>  8);
  const uint8_t b_b = (b >>  0);
  // blend
  const uint8_t o_r = (a_r * (255 - i) + (b_r * i)) >> 8;
  const uint8_t o_g = (a_g * (255 - i) + (b_g * i)) >> 8;
  const uint8_t o_b = (a_b * (255 - i) + (b_b * i)) >> 8;
  // repack
  return uint32_t(o_r << 16) |
         uint32_t(o_g <<  8) |
         uint32_t(o_b <<  0);
#else
  const uint8_t j = 255 - i;
  // pixel a
  const uint32_t a0 = ((a & 0xff00ff) * j) >> 8;
  const uint32_t a1 = ((a & 0x00ff00) * j) >> 8;
  // pixel b
  const uint32_t b0 = ((b & 0xff00ff) * i) >> 8;
  const uint32_t b1 = ((b & 0x00ff00) * i) >> 8;
  // mix results
  return ((a0 & 0xff00ff) | (a1 & 0xff00)) +
         ((b0 & 0xff00ff) | (b1 & 0xff00));
#endif
}

} // namespace {}

namespace tengu {

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

void draw_t::circleAA(
    const vec2i_t& center,
    const int32_t radius)
{
    assert(target_);
    recti_t rect = {
        center.x - radius - 1,
        center.y - radius - 1,
        center.x + radius + 1,
        center.y + radius + 1
    };
    rect = recti_t::intersect(viewport_, rect);
    const float r2 = 1.f / (radius * radius);
    const uint32_t pitch = target_->width();
    uint32_t* pix = target_->data() + rect.y0 * pitch;
    const uint32_t colour = colour_;
    // rasterize loop
    for (int y = rect.y0; y <= rect.y1; ++y) {
        for (int x = rect.x0; x <= rect.x1; ++x) {
            // distance to edge
            const float dx = absv(x - center.x);
            const float dy = absv(y - center.y);
            // distance squared
            const float dx2 = dx * dx;
            const float dy2 = dy * dy;
            // calculate edge distance
            const float v = (dx2 + dy2) * r2;
            const float error = (v - 1.f) * radius;
            // greater then edge
            if (error > 1.f) {
                continue;
            }
            // less than edge
            if (error < -1.f) {
                // skip spans on leading edge
                if (x < center.x) {
                    const int32_t nx = x + (dx * 2);
                    _span(x, nx, y);
                    x = nx;
                } else {
                    pix[x] = colour;
                }
                continue;
            }
            // render edge
            pix[x] = alpha_blend(pix[x], colour, 128 - 127 * error);
        }
        pix += pitch;
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

void draw_t::rect_gliss(const recti_t p)
{
    assert(target_);
    const recti_t rect = recti_t::intersect(viewport_ + recti_t{ 0, 0, 1, 1 }, p);
    const uint32_t pitch = target_->width();
    const uint32_t colour = (colour_ >> 1) & 0x7f7f7f;
    uint32_t* pix = target_->data() + rect.y0 * pitch;
    for (int y = rect.y0; y < rect.y1; ++y) {
        for (int x = rect.x0; x < rect.x1; ++x) {
            pix[x] = ((pix[x] >> 1) & 0x7f7f7f) + colour;
        }
        pix += pitch;
    }
}

void draw_t::outline(const recti_t p)
{
    assert(target_);
    const recti_t rect = recti_t::intersect(viewport_ + recti_t{ 0, 0, 1, 1 }, p);
    const uint32_t pitch = target_->width();
    const uint32_t colour = colour_;
    uint32_t* pix = target_->data() + rect.y0 * pitch;
    if (p.y0 >= viewport_.y0 && p.y0 < viewport_.y1) {
        uint32_t* pix = target_->data() + rect.y0 * pitch;
        for (int x = rect.x0; x < rect.x1; ++x) {
            pix[x] = colour;
        }
    }
    if (p.y1 >= viewport_.y0 && p.y1 < viewport_.y1) {
        uint32_t* pix = target_->data() + (rect.y1 - 1) * pitch;
        for (int x = rect.x0; x < rect.x1; ++x) {
            pix[x] = colour;
        }
    }
    if (p.x0 >= viewport_.x0 && p.x0 < viewport_.x1) {
        uint32_t* pix = target_->data() + rect.y0 * pitch + rect.x0;
        for (int y = rect.y0; y < rect.y1; ++y) {
            pix[0] = colour;
            pix += pitch;
        }
    }
    if (p.x1 >= viewport_.x0 && p.x1 < viewport_.x1) {
        uint32_t* pix = target_->data() + rect.y0 * pitch + (rect.x1 - 1);
        for (int y = rect.y0; y < rect.y1; ++y) {
            pix[0] = colour;
            pix += pitch;
        }
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
    viewport_ = target_size();
}

void draw_t::viewport(const recti_t& rect)
{
    recti_t vp = target_size();
    viewport_ = recti_t::intersect(vp, rect);
}

void draw_t::viewport()
{
    viewport_ = target_size();
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

recti_t draw_t::target_size() const
{
    assert(target_);
    return recti_t{
        0,
        0,
        target_->width() - 1,
        target_->height() - 1
    };
}

// render at 1:1 scale
void draw_t::render_1x(void* mem, const uint32_t pitch)
{
    assert(mem);
    // data access
    uint32_t* dst = reinterpret_cast<uint32_t*>(mem);
    const uint32_t* src = target_->data();
    const uint32_t src_pitch = target_->width();
    // height iterator
    for (int32_t y = 0; y < target_->height(); ++y) {
        // scan lines
        uint32_t* x0 = dst;
        // draw spans
        for (uint32_t i = 0; i < src_pitch; ++i) {
            x0[i] = src[i];
        }
        // advance scan lines
        dst += pitch;
        src += target_->width();
    }
}

// render at 1:2 scale
void draw_t::render_2x(void* mem, const uint32_t pitch)
{
    assert(mem);
    // data access
    uint32_t* dst = reinterpret_cast<uint32_t*>(mem);
    const uint32_t* src = target_->data();
    const uint32_t src_pitch = target_->width();
    // height iterator
    for (int32_t y = 0; y < target_->height(); ++y) {
        // scan lines
        uint32_t* x0 = dst;
        uint32_t* x1 = dst + pitch;
        // draw spans
        for (uint32_t i = 0; i < src_pitch; ++i) {
            const uint32_t rgb = src[i];
            x0[i * 2 + 0] = rgb;
            x0[i * 2 + 1] = rgb;
            x1[i * 2 + 0] = rgb;
            x1[i * 2 + 1] = rgb;
        }
        // advance scan lines
        dst += pitch * 2;
        src += target_->width();
    }
}

// render at 1:3 scale
void draw_t::render_3x(void* mem, const uint32_t pitch)
{
    assert(mem);
    // data access
    uint32_t* dst = reinterpret_cast<uint32_t*>(mem);
    const uint32_t* src = target_->data();
    const uint32_t src_pitch = target_->width();
    // height iterator
    for (int32_t y = 0; y < target_->height(); ++y) {
        // scan lines
        uint32_t* x0 = dst;
        uint32_t* x1 = x0 + pitch;
        uint32_t* x2 = x1 + pitch;
        // draw spans
        for (uint32_t i = 0; i < src_pitch; ++i) {
            const uint32_t rgb = src[i];
            x0[i * 3 + 0] = rgb;
            x0[i * 3 + 1] = rgb;
            x0[i * 3 + 2] = rgb;
            x1[i * 3 + 0] = rgb;
            x1[i * 3 + 1] = rgb;
            x1[i * 3 + 2] = rgb;
            x2[i * 3 + 0] = rgb;
            x2[i * 3 + 1] = rgb;
            x2[i * 3 + 2] = rgb;
        }
        // advance scan lines
        dst += pitch * 3;
        src += target_->width();
    }
}

void draw_t::printf(const font_t& font,
    const vec2i_t& pos,
    const char* fmt,
    ...)
{
    const uint32_t xfit = font.bitmap_->width() / font.cellw_;
    std::array<char, 1024> temp;
    blit_info_t info;
    info.type_ = e_blit_mask;
    info.bitmap_ = font.bitmap_;
    info.h_flip_ = false;
    info.dst_pos_ = vec2i_t{ pos.x, pos.y };
    va_list vl;
    va_start(vl, fmt);
    if (int j = vsnprintf(temp.data(), temp.size(), fmt, vl)) {
        if (j <= 0) {
            return;
        }
        j = minv<int32_t>(int32_t(temp.size()), j);
        for (int i = 0; i < j; i++) {
            const uint8_t ch = temp[i];
            info.src_rect_ = recti_t(
                int32_t((ch % xfit) * font.cellw_),
                int32_t((ch / xfit) * font.cellh_),
                (font.cellw_ - 1),
                (font.cellh_ - 1),
                recti_t::e_relative);
            blit(info);
            info.dst_pos_.x += font.spacing_;
        }
    }
    va_end(vl);
}

} // namespace tengu
