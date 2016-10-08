#pragma once
#include <cstdint>
#include <array>

#include "../framework_core/vec2.h"
#include "../framework_core/vec3.h"
#include "../framework_core/rect.h"
#include "bitmap.h"

namespace tengu {
enum blit_type_t {
    e_blit_opaque,   // no colour key
    e_blit_key,      // colour key
    e_blit_gliss,    // half transparency
    e_blit_mask,     // white masked by key
    e_blit_dither_1, // ordered dither 1
    e_blit_dither_2, // ordered dither 2
    e_blit_dither_3, // ordered dither 3
    e_blit_dither_4, // ordered dither 4
    e_blit_dither_5, // ordered dither 5
};

struct blit_info_t {
    const struct bitmap_t * bitmap_;
    vec2i_t dst_pos_;
    recti_t src_rect_;
    blit_type_t type_;
    bool h_flip_;
};

struct blit_info_ex_t {
    const struct bitmap_t * bitmap_;
    vec2f_t dst_pos_;
    recti_t src_rect_;
    blit_type_t type_;
    std::array<float, 4> matrix_;
};

struct font_t {
    const struct bitmap_t * bitmap_;
    int32_t cellw_, cellh_;
    int32_t spacing_;
};

// <---- ---- ---- ---- todo: add foreground and background colour masking
struct tilemap_t {
    vec2i_t map_size_;
    vec2i_t cell_size_;
    uint8_t * cells_;
    bitmap_t * bitmap_;
    blit_type_t type_;
};

struct draw_t {

    draw_t()
        : viewport_{0, 0, 0, 0}
        , target_(nullptr)
    {
    }

    void clear();

    void circle(const vec2i_t & p,
                const int32_t radius);

    void rect(const recti_t p);

    void triangle(const vec2f_t & a,
                  const vec2f_t & b,
                  const vec2f_t & c);

    void line(const vec2f_t & p0,
              const vec2f_t & p1);

    void line(const vec2i_t & p0,
              const vec2i_t & p1);

    void plot(const vec2i_t & p);

    void blit(const blit_info_t & info);

    void blit(const blit_info_ex_t & info);

    void blit(const tilemap_t & tiles, vec2i_t & p);

    void set_target(struct bitmap_t &);

    void viewport(const recti_t & rect);

    void viewport();

    void copy(struct bitmap_t & dst,
              const recti_t & src_rect,
              const vec2i_t & dst_pos);

    void printf(const font_t &,
                const vec2i_t & pos,
                const char * fmt, ...);

    bitmap_t & get_target() {
        return *target_;
    }

    void render_3x(void *dst,
                   const uint32_t pitch);

    void render_2x(void *dst,
                   const uint32_t pitch);

    void render_1x(void *dst,
                   const uint32_t pitch);

    uint32_t colour_, key_;

protected:

    void _span(int32_t x0,
               int32_t x1,
               int32_t y);

    recti_t _target_size() const;

    recti_t viewport_;
    bitmap_t * target_;
};

struct draw_ex_t {
protected:
    template <bool C_OFFSET, typename vec_t>
    vec_t select(const vec_t & vec) {
        return C_OFFSET ? vec+vec_t(offset_) : vec;
    }

    template <bool C_OFFSET, typename type_t>
    type_t select_x(const type_t & val) {
        return C_OFFSET ? val+offset_.x : val;
    }

    template <bool C_OFFSET, typename type_t>
    type_t select_y(const type_t & val) {
        return C_OFFSET ? val+offset_.y : val;
    }

public:
    draw_ex_t(draw_t & draw)
        : draw_(draw)
        , colour_(draw.colour_)
        , key_(draw.key_)
        , offset_{0, 0}
    {
    }

    template <bool C_OFFSET = true>
    void rect(const recti_t p) {
        draw_.rect(recti_t{
            select_x<C_OFFSET>(p.x0),
            select_y<C_OFFSET>(p.y0),
            select_x<C_OFFSET>(p.x1),
            select_y<C_OFFSET>(p.y1)
        });
    }

    template <bool C_OFFSET = true>
    void circle(const vec2i_t & p,
                const int32_t radius) {
        draw_.circle(select<C_OFFSET, vec2i_t>(p), radius);
    }

    template <typename vec_t, bool C_OFFSET = true>
    void line(const vec_t & p0,
              const vec_t & p1) {
        draw_.line(select<C_OFFSET, vec_t>(p0),
                   select<C_OFFSET, vec_t>(p1));
    }

    template <bool C_OFFSET = true>
    void plot(const vec2i_t & p) {
        draw_.plot(select<C_OFFSET>(p));
    }

    template <bool C_OFFSET = true>
    void triangle(const vec2f_t & a,
                  const vec2f_t & b,
                  const vec2f_t & c) {
        draw_.triangle(select<C_OFFSET>(a),
                       select<C_OFFSET>(b),
                       select<C_OFFSET>(c));
    }

    template <bool C_OFFSET = true>
    void blit(const blit_info_t & info) {
        blit_info_t info_b = info;
        info_b.dst_pos_ = select<C_OFFSET>(info.dst_pos_);
        draw_.blit(info_b);
    }

    template <bool C_OFFSET = true>
    void blit(const blit_info_ex_t & info) {
        blit_info_ex_t info_b = info;
        info_b.dst_pos_ = select<C_OFFSET>(info.dst_pos_);
        draw_.blit(info_b);
    }

    draw_t & draw_;
    uint32_t & colour_;
    uint32_t & key_;
    vec2i_t offset_;
};
} // namespace tengu
