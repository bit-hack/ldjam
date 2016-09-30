#include <cstdarg>
#include <cstdio>
#include <array>

#include "draw.h"
#include "../framework_core/common.h"

constexpr uint8_t bin8(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                       uint8_t e, uint8_t f, uint8_t g, uint8_t h) {
    return (a<<7) | (b<<6) | (c<<5) | (d<<4) | (e<<3) | (f<<2) | (g<<1) | h;
}

namespace {
const std::array<uint32_t, 5> _dither = {
     0x44001100u, // dither 1
     0x55005500u, // dither 2
     0x55aa55aau, // dither 3
    ~0x55005500u, // dither 4
    ~0x44001100u, // dither 5
};

template <typename typea_t, typename typeb_t>
float orient2d(const vec2_t<typea_t> & a,
               const vec2_t<typea_t> & b,
               const vec2_t<typeb_t> & c) {
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}

void _draw_t_clip(const recti_t & viewport,
                  recti_t &src,
                  recti_t &dst) {
    const recti_t & vp = viewport;
    // top left clip amount
    const int32_t lx = max2(vp.x0-dst.x0, 0);
    const int32_t ly = max2(vp.y0-dst.y0, 0);
    // adjust source and destination
    src.x0 += lx;
    src.y0 += ly;
    dst.x0 += lx;
    dst.y0 += ly;
    // clip lower right
    dst.x1 = min2(vp.x1, dst.x1);
    dst.y1 = min2(vp.y1, dst.y1);
}

// pixel blending function type
typedef uint32_t pixel_func_t(const uint32_t src,
                              const uint32_t dest,
                              const uint32_t colour,
                              const uint32_t key,
                              const uint32_t order);

template <pixel_func_t mode_t, bool hflip = false>
void _draw_t_blit_2(bitmap_t & target,
                    const recti_t & viewport,
                    const blit_info_t & info,
                    const uint32_t colour,
                    const uint32_t key) {
    // calculate dest rect
    recti_t dst_rect(
            info.dst_pos_.x,
            info.dst_pos_.y,
            info.src_rect_.dx(),
            info.src_rect_.dy(),
            recti_t::e_relative);
    // quickly classify sprite on viewport
    recti_t::classify_t c = viewport.classify(dst_rect);
    if (c == recti_t::e_rect_outside) {
        return;
    }
    // clip src rect to the source bitmap
    recti_t src_rect = recti_t::intersect(info.src_rect_, info.bitmap_->rect());
    // clip to the viewport
    if (c == recti_t::e_rect_overlap) {
        _draw_t_clip(viewport, src_rect, dst_rect);
    }
    //
    if (hflip) {
        const int32_t xshift = src_rect.x0 - info.src_rect_.x0;
        src_rect.x1 -= xshift;
    }
    // destination buffer setup
    const int32_t dst_pitch = target.width();
    uint32_t * dst =
            target.data() +
            dst_rect.x0 +
            dst_rect.y0 * dst_pitch;
    // src buffer setup
    const int32_t src_pitch = info.bitmap_->width();
    const uint32_t * src =
            info.bitmap_->data() +
            src_rect.x0 +
            src_rect.y0 * src_pitch;
    // dither nudge
    const int32_t xn = dst_rect.x0 & 1;
    const int32_t yn = dst_rect.y0 & 1;
    // main blitter loop
    for (int32_t y = 0; y <= dst_rect.dy(); y++) {
        for (int32_t x = 0; x <= dst_rect.dx(); x++) {
            // dither order
            const uint32_t dither = ((x^xn)&0x7) + ((y^yn)<<3);
            // invoke blend function
            dst[x] = mode_t(src[hflip ? src_rect.dx() - x : x],
                            dst[x],
                            colour,
                            key,
                            dither);
        }
        dst += dst_pitch;
        src += src_pitch;
    }
}

// opaque blend mode (100% src, no key)
constexpr uint32_t mode_opaque(const uint32_t src,
                               const uint32_t,
                               const uint32_t,
                               const uint32_t,
                               const uint32_t) {
    return src;
}

// colour key blend mode
constexpr uint32_t mode_key(const uint32_t src,
                            const uint32_t dst,
                            const uint32_t,
                            const uint32_t key,
                            const uint32_t) {
    return (src == key) ?
           dst :
           src;
}

// 50% transparency
constexpr uint32_t mode_gliss(const uint32_t src,
                              const uint32_t dst,
                              const uint32_t,
                              const uint32_t key,
                              const uint32_t) {
    return (src == key) ?
           (dst) :
           ((src>>1)&0x7f7f7f) + ((dst>>1)&0x7f7f7f);
}

// colour keyed overlay
constexpr uint32_t mode_mask(const uint32_t src,
                             const uint32_t dst,
                             const uint32_t colour,
                             const uint32_t key,
                             const uint32_t) {
    return (src == key) ?
           (dst) :
           (colour);
}

// ordered dither
template <int32_t _ORDER>
constexpr uint32_t mode_dither(const uint32_t src,
                               const uint32_t dst,
                               const uint32_t colour,
                               const uint32_t key,
                               const uint32_t order) {
    // ordered dither
    return (src == key || (_dither[_ORDER]&(1 << order)) == 0) ?
           (dst) :
           (src);
}

template <bool hflip>
void _draw_t_blit_1(bitmap_t & target,
                    const recti_t & viewport,
                    const blit_info_t & info,
                    const uint32_t colour,
                    const uint32_t key) {
    // blit function prototype
    typedef void (*blit_func_t)(bitmap_t & target,
                                const recti_t & viewport,
                                const blit_info_t & info,
                                const uint32_t colour,
                                const uint32_t key);
    // blend mode table
    // must be kept in order of blit_type_t
    static const std::array<blit_func_t, 9> func = {{
        _draw_t_blit_2<mode_opaque, hflip>,
        _draw_t_blit_2<mode_key, hflip>,
        _draw_t_blit_2<mode_gliss, hflip>,
        _draw_t_blit_2<mode_mask, hflip>,
        _draw_t_blit_2<mode_dither<0>, hflip>,
        _draw_t_blit_2<mode_dither<1>, hflip>,
        _draw_t_blit_2<mode_dither<2>, hflip>,
        _draw_t_blit_2<mode_dither<3>, hflip>,
        _draw_t_blit_2<mode_dither<4>, hflip>
    }};
    // dispatch to blit function
    assert(info.type_ < func.size());
    func[info.type_](target, viewport, info, colour, key);
};
} // namespace {}


void draw_t::clear() {
    assert(target_);
    const uint32_t pitch = target_->width();
    const uint32_t colour = colour_;
    recti_t viewport = viewport_;
    uint32_t * pix = target_->data() + viewport.y0 * pitch;
    for (int y=viewport.y0; y<=viewport.y1; ++y) {
        _span(viewport.x0, viewport.x1, y);
        pix += pitch;
    }
}

void draw_t::circle(
    const vec2i_t & center,
    const int32_t radius) {
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
    const recti_t rect = recti_t::intersect(viewport_+recti_t{0, 0, 1, 1}, p);
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
    const vec2f_t & v0,
    const vec2f_t & v1,
    const vec2f_t & v2) {
    // triangle bounds
    int32_t minx = int32_t(min3(v0.x, v1.x, v2.x));
    int32_t maxx = int32_t(max3(v0.x, v1.x, v2.x));
    int32_t miny = int32_t(min3(v0.y, v1.y, v2.y));
    int32_t maxy = int32_t(max3(v0.y, v1.y, v2.y));
    // clip min point to screen
    minx = max2<int32_t>(minx+0, viewport_.x0);
    miny = max2<int32_t>(miny+0, viewport_.y0);
    maxx = min2<int32_t>(maxx+1, viewport_.x1);
    maxy = min2<int32_t>(maxy+1, viewport_.y1);
    // the signed triangle area
    const float area = 1.f / ((v1.x - v0.x)*(v2.y - v0.y)-
                              (v2.x - v0.x)*(v1.y - v0.y));
    // reject back faces
    if (area <= 0.f)
        return;
    // barycentric x step
    const vec3f_t sx = {
        (v0.y - v1.y) * area, // A01
        (v1.y - v2.y) * area, // A12
        (v2.y - v0.y) * area  // A20
    };
    // barycentric y step
    const vec3f_t sy = {
        (v1.x - v0.x) * area, // B01
        (v2.x - v1.x) * area, // B12
        (v0.x - v2.x) * area  // B20
    };
    // barycentric start point
    vec2i_t p = { minx, miny };
    // barycentric value at start point
    vec3f_t wy = {
        orient2d(v0, v1, p) * area, // W2 -> A01, B01
        orient2d(v1, v2, p) * area, // W0 -> A12, B12
        orient2d(v2, v0, p) * area, // W1 -> A20, B20
    };
    const uint32_t dst_pitch = target_->width();
    const uint32_t colour = colour_;
    uint32_t * pix = target_->data() + p.y * dst_pitch;
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

void draw_t::line(
    const vec2f_t & p0,
    const vec2f_t & p1) {
    line(vec2i_t{int32_t(p0.x), int32_t(p0.y)},
         vec2i_t{int32_t(p1.x), int32_t(p1.y)});
}

void draw_t::line(
    const vec2i_t & p0,
    const vec2i_t & p1) {

    // <---- ---- ---- ---- ---- ---- ---- ---- todo: clip line to viewport
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
    const int32_t decInc = (longLen == 0) ? 0 : (shortLen << 16) / longLen;
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
    // dispatch based on hflip
    if (info.h_flip_) {
        _draw_t_blit_1<true>(*target_, viewport_, info, colour_, key_);
    }
    else {
        _draw_t_blit_1<false>(*target_, viewport_, info, colour_, key_);
    }
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
        x0 = max2(x0, viewport_.x0);
        x1 = min2(x1, viewport_.x1);
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
    for (int32_t y=0; y<target_->height(); ++y) {
        // scan lines
        uint32_t * x0 = dst;
        uint32_t * x1 = dst + pitch;
        // draw spans
        for (uint32_t i = 0; i < src_pitch; ++i) {
            const uint32_t rgb = src[i];
            x0[i * 2 + 0] = rgb; x0[i * 2 + 1] = rgb;
            x1[i * 2 + 0] = rgb; x1[i * 2 + 1] = rgb;
        }
        // advance scan lines
        dst += pitch * 2;
        src += target_->width();
    }
}

void draw_t::render_3x(void * mem, const uint32_t pitch) {
    assert(mem);
    // data access
    uint32_t * dst = reinterpret_cast<uint32_t*>(mem);
    const uint32_t * src = target_->data();
    const uint32_t src_pitch = target_->width();
    // height iterator
    for (int32_t y=0; y<target_->height(); ++y) {
        // scan lines
        uint32_t * x0 = dst;
        uint32_t * x1 = x0 + pitch;
        uint32_t * x2 = x1 + pitch;
        // draw spans
        for (uint32_t i = 0; i < src_pitch; ++i) {
            const uint32_t rgb = src[i];
            x0[i * 3 + 0] = rgb; x0[i * 3 + 1] = rgb; x0[i * 3 + 2] = rgb;
            x1[i * 3 + 0] = rgb; x1[i * 3 + 1] = rgb; x1[i * 3 + 2] = rgb;
            x2[i * 3 + 0] = rgb; x2[i * 3 + 1] = rgb; x2[i * 3 + 2] = rgb;
        }
        // advance scan lines
        dst += pitch * 3;
        src += target_->width();
    }
}

void draw_t::render_1x(void * mem, const uint32_t pitch) {
    assert(mem);
    // data access
    uint32_t * dst = reinterpret_cast<uint32_t*>(mem);
    const uint32_t * src = target_->data();
    const uint32_t src_pitch = target_->width();
    // height iterator
    for (int32_t y=0; y<target_->height(); ++y) {
        // scan lines
        uint32_t * x0 = dst;
        // draw spans
        for (uint32_t i = 0; i < src_pitch; ++i) {
            x0[i] = src[i];
        }
        // advance scan lines
        dst += pitch;
        src += target_->width();
    }
}

void draw_t::printf(const font_t & font,
                    const vec2i_t & pos,
                    const char * fmt,
                    ...) {
    const uint32_t xfit = font.bitmap_->width() / font.cellw_;
    std::array<char, 1024> temp;
    blit_info_t info;
    info.type_ = e_blit_mask;
    info.bitmap_ = font.bitmap_;
    info.h_flip_ = false;
    info.dst_pos_ = vec2i_t{ pos.x, pos.y };
    va_list vl;
    va_start(vl, fmt);
    if (int j=vsnprintf(temp.data(), temp.size(), fmt, vl)) {
        if (j <= 0) {
            return;
        }
        j = min2<int32_t>(int32_t(temp.size()), j);
        for (int i=0; i<j; i++) {
            const uint8_t ch = temp[i];
            info.src_rect_ = recti_t(
                int32_t((ch % xfit) * font.cellw_),
                int32_t((ch / xfit) * font.cellh_),
                (font.cellw_ - 1),
                (font.cellh_ - 1),
                recti_t::e_relative
            );
            blit(info);
            info.dst_pos_.x += font.spacing_;
        }
    }
    va_end(vl);
}

void draw_t::blit(const tilemap_t & tiles, vec2i_t & p) {
    // cell size
    const int32_t cell_w = tiles.cell_size_.x;
    const int32_t cell_h = tiles.cell_size_.y;
    // cells fitting into sprite sheet x axis
    const int32_t cells_w =
            tiles.bitmap_->width() / tiles.cell_size_.x;
    // viewport to map space
    int32_t x0 = clampv(0, quantize(viewport_.x0-p.x, cell_w), tiles.map_size_.x-1);
    int32_t y0 = clampv(0, quantize(viewport_.y0-p.y, cell_h), tiles.map_size_.y-1);
    int32_t x1 = clampv(0, quantize(viewport_.x1-p.x, cell_w), tiles.map_size_.x-1);
    int32_t y1 = clampv(0, quantize(viewport_.y1-p.y, cell_h), tiles.map_size_.y-1);
    // blank blit into
    blit_info_t info = {
        tiles.bitmap_,
        p,
        recti_t(),
        tiles.type_,
        false
    };
    // find first blit point
    vec2i_t pos{p.x + x0 * cell_w, p.y + y0 * cell_h};
    // main blit loop
    for (int32_t y=y0; y<=y1; ++y) {

        info.dst_pos_ = pos;

        for (int32_t x=x0; x<=x1; ++x) {
            // 
            const uint8_t cell =
                    tiles.cells_[x + y * tiles.map_size_.x];
            // calculate source rectangle
            info.src_rect_ = recti_t(
                (cell % cells_w) * tiles.cell_size_.x,
                (cell / cells_w) * tiles.cell_size_.y,
                cell_w - 1,
                cell_h - 1,
                recti_t::e_relative
            );
            // blit this tile
            blit(info);
            // advance along this row
            info.dst_pos_.x += cell_w;
        }
        // wrap and step one column
        pos.y += cell_h;
    }
}

void draw_t::blit(const blit_info_ex_t & info) {
    // alias
    const auto & src = info.src_rect_;
    const auto & dst = info.dst_pos_;
    const auto & mat = info.matrix_;
    // rotated aabb size
    // <---- this is broken/borked/wrong
    const float aabb_dx = (absv(mat[0] * src.dx() + mat[1] * src.dy())) / 2;
    const float aabb_dy = (absv(mat[2] * src.dx() + mat[3] * src.dy())) / 2;
    // starting read position
    // midpoint  - left corner
    float rx = (src.x0 + src.x1) / 2 - (mat[0] * aabb_dx + mat[1] * aabb_dy) / 2;
    float ry = (src.y0 + src.y1) / 2 - (mat[2] * aabb_dx + mat[3] * aabb_dy) / 2;
    // starting write position
    recti_t rect = {
        int32_t(dst.x - aabb_dx),
        int32_t(dst.y - aabb_dy),
        int32_t(dst.x + aabb_dx),
        int32_t(dst.y + aabb_dy),
    };
    // clip to viewport
    // discover starting read
    // y axis iteration
    for (int32_t y = rect.y0; y < rect.y1; ++y) {
        float tx = rx;
        float ty = ry;
        // x axis iteration
        for (int32_t x = rect.x0; x < rect.x1; ++x) {
            // if the
            if (src.contains(vec2i_t{int32_t(tx),
                                     int32_t(ty)})) {
                colour_ = 0xff00ff;
            }
            else {
                colour_ = 0xff00;
            }
            // plot pixel
            plot(vec2i_t{x, y});
            // step x
            tx += mat[0];
            ty += mat[1];
        }
        // step y
        rx += mat[2];
        ry += mat[3];
    }
}
