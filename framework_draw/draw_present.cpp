#include <array>
#include <cstdarg>
#include <cstdio>

#include "../framework_core/common.h"
#include "draw.h"

namespace tengu {

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
} // namespace tengu
