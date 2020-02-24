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

template <typename typea_t, typename typeb_t>
float _orient2d(const vec2_t<typea_t>& a,
    const vec2_t<typea_t>& b,
    const vec2_t<typeb_t>& c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

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

// 1:1 sprite blit function
template <bool hflip, pixel_func_t mode_t>
void _draw_t_blit_2(bitmap_t& target,
    const recti_t& viewport,
    const blit_info_t& info,
    const uint32_t colour,
    const uint32_t key)
{
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
    // if we are horozontal flipping
    if (hflip) {
        const int32_t xshift = src_rect.x0 - info.src_rect_.x0;
        src_rect.x1 -= xshift;
    }
    // destination buffer setup
    const int32_t dst_pitch = target.width();
    uint32_t* dst = target.data() + dst_rect.x0 + dst_rect.y0 * dst_pitch;
    // src buffer setup
    const int32_t src_pitch = info.bitmap_->width();
    const uint32_t* src = info.bitmap_->data() + src_rect.x0 + src_rect.y0 * src_pitch;
    // dither nudge
    const int32_t xn = dst_rect.x0 & 1;
    const int32_t yn = dst_rect.y0 & 1;
    // main blitter loop
    for (int32_t y = 0; y <= dst_rect.dy(); y++) {
        for (int32_t x = 0; x <= dst_rect.dx(); x++) {
            // dither order
            const uint32_t dither = ((x ^ xn) & 0x7) + ((y ^ yn) << 3);
            const uint32_t sc = src[hflip ? src_rect.dx() - x : x];
            // invoke blend function
            dst[x] = mode_t(sc, /* source */
                dst[x], /* dest */
                colour, /* colour */
                key, /* key */
                dither); /* dither */
        }
        dst += dst_pitch;
        src += src_pitch;
    }
}

vec2f_t _rotate(const std::array<float, 4>& mat,
    const vec2f_t& in)
{
    return vec2f_t{
        in.x * mat[0] + in.y * mat[2],
        in.x * mat[1] + in.y * mat[3]
    };
}

// rotospite blit function
template <pixel_func_t func>
void _draw_t_blit_2(bitmap_t& target,
    const recti_t& viewport,
    const blit_info_ex_t& info,
    const uint32_t colour,
    const uint32_t key)
{
    // alias
    const auto& src = info.src_rect_;
    const auto& dst = info.dst_pos_;
    const auto& mat = info.matrix_;
    // invert 2D matrix
    const float det = 1.f / (mat[0] * mat[3] - mat[1] * mat[2]);
    const std::array<float, 4> imat = {
        det * mat[3], -det * mat[1],
        -det * mat[2], det * mat[0]
    };
    // source pos at mid point
    const float sx = float(src.x0 + src.x1) * .5f;
    const float sy = float(src.y0 + src.y1) * .5f;
    // sprite size
    const vec2f_t size = vec2f_t{
        float(src.dx()) * .5f,
        float(src.dy()) * .5f
    };
    // find edge points
    const vec2f_t p1 = dst + _rotate(imat, vec2f_t{ float(-src.width()), float(-src.height()) }) * .5f;
    const vec2f_t p2 = dst + _rotate(imat, vec2f_t{ float(src.width()), float(-src.height()) }) * .5f;
    const vec2f_t p3 = dst + _rotate(imat, vec2f_t{ float(-src.width()), float(src.height()) }) * .5f;
    const vec2f_t p4 = dst + _rotate(imat, vec2f_t{ float(src.width()), float(src.height()) }) * .5f;
    // rotated sprites aabb
    const recti_t dst_rect = recti_t::intersect(
        viewport,
        recti_t{ int32_t(minv(p1.x, p2.x, p3.x, p4.x)),
            int32_t(minv(p1.y, p2.y, p3.y, p4.y)),
            int32_t(maxv(p1.x, p2.x, p3.x, p4.x) + 1),
            int32_t(maxv(p1.y, p2.y, p3.y, p4.y) + 1) });
    // top left source pos
    const float mx = float(dst_rect.x0) - dst.x;
    const float my = float(dst_rect.y0) - dst.y;
    float rx = sx + (mx * mat[0] + my * mat[2]);
    float ry = sy + (mx * mat[1] + my * mat[3]);
    // src buffer setup
    const int32_t src_pitch = info.bitmap_->width();
    const uint32_t* src_pix = info.bitmap_->data();
    // destination buffer setup
    const int32_t dst_pitch = target.width();
    uint32_t* dst_pix = target.data() + dst_rect.y0 * dst_pitch;
    // dither nudge
    const int32_t xn = dst_rect.x0 & 1;
    const int32_t yn = dst_rect.y0 & 1;
    // y axis iteration
    for (int32_t y = dst_rect.y0; y <= dst_rect.y1; ++y) {
        float tx = rx;
        float ty = ry;
        // x axis iteration
        for (int32_t x = dst_rect.x0; x <= dst_rect.x1; ++x) {
            // if we are inside the source rect
            if (src.contains(vec2i_t{ int32_t(tx), int32_t(ty) })) {
                // source pixel
                const uint32_t sc = src_pix[int32_t(tx) + int32_t(ty) * src_pitch];
                // dither order
                const uint32_t dither = ((x ^ xn) & 0x7) + ((y ^ yn) << 3);
                // plot pixel
                dst_pix[x] = func(sc, /* source */
                    dst_pix[x], /* dest */
                    colour, /* colour */
                    key, /* key */
                    dither); /* dither */
            }
            // step x
            tx += mat[0];
            ty += mat[1];
        }
        // step y
        rx += mat[2];
        ry += mat[3];
        //
        dst_pix += dst_pitch;
    }
}

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

// 1:1 sprite blit (blend dispatch)
template <bool hflip>
void _draw_t_blit_1(bitmap_t& target,
    const recti_t& viewport,
    const blit_info_t& info,
    const uint32_t colour,
    const uint32_t key)
{
    // blit function prototype
    typedef void (*blit_func_t)(bitmap_t & target,
        const recti_t& viewport,
        const blit_info_t& info,
        const uint32_t colour,
        const uint32_t key);
    // blend mode table
    // note: must be kept in order of blit_type_t
    static const std::array<blit_func_t, 9> func = { { _draw_t_blit_2<hflip, _blend_opaque>,
        _draw_t_blit_2<hflip, _blend_key>,
        _draw_t_blit_2<hflip, _blend_gliss>,
        _draw_t_blit_2<hflip, _blend_mask>,
        _draw_t_blit_2<hflip, _blend_dither<0>>,
        _draw_t_blit_2<hflip, _blend_dither<1>>,
        _draw_t_blit_2<hflip, _blend_dither<2>>,
        _draw_t_blit_2<hflip, _blend_dither<3>>,
        _draw_t_blit_2<hflip, _blend_dither<4>> } };
    // dispatch to blit function
    assert(size_t(info.type_) < func.size());
    func[info.type_](target, viewport, info, colour, key);
};
} // namespace {}

// 1:1 sprite blit (hflip dispatch)
void draw_t::blit(const blit_info_t& info)
{
    assert(target_ && info.bitmap_->valid());
    // dispatch based on hflip
    if (info.h_flip_) {
        _draw_t_blit_1<true>(*target_, viewport_, info, colour_, key_);
    } else {
        _draw_t_blit_1<false>(*target_, viewport_, info, colour_, key_);
    }
}

// tilemap blit function
void draw_t::blit(const tilemap_t& tiles, const vec2i_t& p)
{
    // cell size
    const int32_t cell_w = tiles.cell_size_.x;
    const int32_t cell_h = tiles.cell_size_.y;
    // cells fitting into sprite sheet x axis
    const int32_t cells_w = tiles.bitmap_->width() / tiles.cell_size_.x;
    // viewport to map space
    int32_t x0 = clampv(0, quantize(viewport_.x0 - p.x, cell_w), tiles.map_size_.x - 1);
    int32_t y0 = clampv(0, quantize(viewport_.y0 - p.y, cell_h), tiles.map_size_.y - 1);
    int32_t x1 = clampv(0, quantize(viewport_.x1 - p.x, cell_w), tiles.map_size_.x - 1);
    int32_t y1 = clampv(0, quantize(viewport_.y1 - p.y, cell_h), tiles.map_size_.y - 1);
    // blank blit into
    blit_info_t info = {
        tiles.bitmap_,
        p,
        recti_t(),
        tiles.type_,
        false
    };
    // find first blit point
    vec2i_t pos{ p.x + x0 * cell_w, p.y + y0 * cell_h };
    // main blit loop
    for (int32_t y = y0; y <= y1; ++y) {

        info.dst_pos_ = pos;

        for (int32_t x = x0; x <= x1; ++x) {
            //
            const uint8_t cell = tiles.cells_[x + y * tiles.map_size_.x];
            // calculate source rectangle
            info.src_rect_ = recti_t(
                (cell % cells_w) * tiles.cell_size_.x,
                (cell / cells_w) * tiles.cell_size_.y,
                cell_w - 1,
                cell_h - 1,
                recti_t::e_relative);
            // blit this tile
            blit(info);
            // advance along this row
            info.dst_pos_.x += cell_w;
        }
        // wrap and step one column
        pos.y += cell_h;
    }
}

// rotosprite blit
void draw_t::blit(const blit_info_ex_t& info)
{
    assert(info.bitmap_ && info.bitmap_->valid());
    // blit function prototype
    typedef void (*blit_func_t)(bitmap_t & target,
        const recti_t& viewport,
        const blit_info_ex_t& info,
        const uint32_t colour,
        const uint32_t key);
    // blend mode table
    // must be kept in order of blit_type_t
    static const std::array<blit_func_t, 9> func = { { _draw_t_blit_2<_blend_opaque>,
        _draw_t_blit_2<_blend_key>,
        _draw_t_blit_2<_blend_gliss>,
        _draw_t_blit_2<_blend_mask>,
        _draw_t_blit_2<_blend_dither<0>>,
        _draw_t_blit_2<_blend_dither<1>>,
        _draw_t_blit_2<_blend_dither<2>>,
        _draw_t_blit_2<_blend_dither<3>>,
        _draw_t_blit_2<_blend_dither<4>> } };
    // dispatch to blit function
    assert(size_t(info.type_) < func.size());
    func[info.type_](*target_, viewport_, info, colour_, key_);
}
} // namespace tengu
