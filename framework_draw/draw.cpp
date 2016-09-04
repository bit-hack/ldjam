#include "draw.h"
#include "../framework_core/common.h"


void draw_t::clear() {
    assert(target_);
    const uint32_t pitch = target_->width();
    const uint32_t colour = colour_;
    recti_t viewport = viewport_;
    uint32_t * pix = target_->data() + viewport.y0 * pitch;
    for (int y=viewport.y0; y<viewport.y1; ++y) {
        for (int x=viewport.x0; x<viewport.x1; ++x) {
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
    const vec3f_t & b,
    const vec3f_t & c) {
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
}

void draw_t::set_target(struct bitmap_t & bmp) {
    target_ = &bmp;
    viewport_ = _target_size();
}

void draw_t::viewport(const recti_t & rect) {
    recti_t vp = _target_size();
    viewport_ = recti_t::intersect(vp, rect);
}

void draw_t::viewport_clear() {
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
        x1 = minv(x0, viewport_.x1);
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
