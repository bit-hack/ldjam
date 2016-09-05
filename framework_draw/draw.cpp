#include "draw.h"
#include "../framework_core/common.h"


void draw_t::clear() {
    assert(target_);
    const uint32_t pitch = target_->width();
    const uint32_t colour = colour_;
    recti_t viewport = viewport_;
    uint32_t * pix = target_->data() + viewport.y0 * pitch;
    for (int y=viewport.y0; y<=viewport.y1; ++y) {
        for (int x=viewport.x0; x<=viewport.x1; ++x) {
            pix[x] = colour;
        }
        pix += pitch;
    }
}

void draw_t::circle(
    const vec2i_t & center,
    const int32_t radius)
{
    assert(target_);
    int32_t xC = center.x, yC = center.y;
    int32_t p = 1 - radius, x = 0, y = radius;
    _span(xC - y, xC + y, yC);
    while (x++ <= y) {
        if (p<0) {
            p += 2 * x + 1;
        }
        else {
            p += 2 * (x - y) + 1;
            y--;
        }
        _span(xC - x, xC + x, yC + y);
        _span(xC - x, xC + x, yC - y);
        _span(xC - y, xC + y, yC + x);
        _span(xC - y, xC + y, yC - x);
    }
}

void draw_t::rect(const recti_t p) {
    assert(target_);
    const recti_t rect = recti_t::intersect(viewport_, p);
    const uint32_t pitch = target_->width();
    const uint32_t colour = colour_;
    uint32_t * pix = target_->data() + rect.y0 * pitch;
    for (int y=rect.y0; y<rect.y1; ++y) {
        for (int x=rect.x0; x<rect.x1; ++x) {
            pix[x] = colour;
        }
        pix += pitch;
    }
}

void draw_t::triangle(
    const vec2f_t & a,
    const vec2f_t & b,
    const vec2f_t & c) {
}

void draw_t::triangle(const triangle_t &) {
}

void draw_t::line(
    const vec2f_t & p0,
    const vec2f_t & p1) {

    bool yLonger = false;
    int32_t x=p0.x, y=p0.y;
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
    }
    else {
        incrementVal = 1;
    }
    int32_t decInc = (longLen == 0) ? 0 : (shortLen << 16) / longLen;
    if (yLonger) {
        for (int32_t i = 0, j = 0; i != endVal; i += incrementVal, j += decInc) {
            plot(vec2i_t{x + (j >> 16), y + i});
        }
    }
    else {
        for (int32_t i = 0, j = 0; i != endVal; i += incrementVal, j += decInc) {
            plot(vec2i_t{x + i, y + (j >> 16)});
        }
    }
}

void draw_t::plot(const vec2i_t & p) {
    assert(target_);
    if (viewport_.contains(p)) {
        const uint32_t colour = colour_;
        target_->data()[p.x + p.y * target_->width()] = colour;
    }
}

void draw_t::blit(const blit_info_t & info) {
    assert(target_ && info.bitmap_->valid());
    // calculate dest rect
    recti_t dst_rect{
        info.dst_pos_.x,
        info.dst_pos_.y,
        info.dst_pos_.x + info.src_rect_.dx(),
        info.dst_pos_.y + info.src_rect_.dy()};
    // quickly classify sprite on viewport
    recti_t::classify_t c = viewport_.classify(dst_rect);
    if (c == recti_t::e_rect_outside)
        return;
    // clip if we need to
    recti_t src_rect = info.src_rect_;
    if (c == recti_t::e_rect_overlap) {
        _clip(src_rect, dst_rect);
#if 0
        return;
#endif
    }
    // dest buffer setup
    const uint32_t dst_pitch = target_->width();
    uint32_t * dst =
        target_->data() +
        dst_rect.x0 +
        dst_rect.y0 * dst_pitch;
    // src buffer setup
    const uint32_t src_pitch = info.bitmap_->width();
    uint32_t * src =
        info.bitmap_->data() +
        src_rect.x0 +
        src_rect.y0 * src_pitch;
    //
    for (int32_t y = 0; y <= dst_rect.dy(); y++) {
        for (int32_t x = 0; x <= dst_rect.dx(); x++) {
            dst[x] = src[x];
        }
        dst += dst_pitch;
        src += src_pitch;
    }
}

void draw_t::_clip(recti_t &src, recti_t &dst) {
    // todo
}

void draw_t::set_target(struct bitmap_t & bmp) {
    target_ = &bmp;
    viewport_ = _target_size();
}

void draw_t::viewport(const recti_t & rect) {
    recti_t vp = _target_size();
    viewport_ = recti_t::intersect(vp, rect);
}

void draw_t::viewport() {
    viewport_ = _target_size();
}

void draw_t::copy(
    struct bitmap_t & dst,
    const recti_t & src_rect,
    const vec2i_t & dst_pos) {
}

void draw_t::_span(int32_t x0, int32_t x1, int32_t y) {
    if (y >= viewport_.y0 && y <= viewport_.y1) {
        x0 = maxv(x0, viewport_.x0);
        x1 = minv(x1, viewport_.x1);
        uint32_t * pix = target_->data();
        pix += y * target_->width();
        const uint32_t colour = colour_;
        for (int32_t x = x0; x <= x1; ++x) {
            pix[x] = colour;
        }
    }
}

recti_t draw_t::_target_size() const {
    assert(target_);
    return recti_t{
        0,
        0,
        target_->width()-1,
        target_->height()-1
    };
}

void draw_t::render_2x(void * mem, const uint32_t pitch) {
    assert(mem);
    // data access
    uint32_t * dst = reinterpret_cast<uint32_t*>(mem);
    const uint32_t * src = target_->data();
    const uint32_t src_pitch = target_->width();
    // height iterator
    for (uint32_t y=0; y<target_->height(); ++y) {
        // scan lines
        uint32_t * x0 = dst;
        uint32_t * x1 = dst + pitch;
        // draw spans
        for (uint32_t i = 0; i < src_pitch; ++i) {
            // pixel colour
            const uint32_t rgb = src[i];
            // upper
            x0[i * 2 + 0] = rgb;
            x0[i * 2 + 1] = rgb;
            // lower
            x1[i * 2 + 0] = rgb;
            x1[i * 2 + 1] = rgb;
        }
        // advance scan lines
        dst += pitch * 2;
        src += target_->width();
    }
}
