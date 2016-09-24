#pragma once
#include <cstdint>
#include "../framework_core/vec2.h"
#include "../framework_core/vec3.h"
#include "../framework_core/rect.h"
#include "bitmap.h"


enum blit_type_t {
    e_blit_opaque,  // no colour key
    e_blit_key,     // colour key
    e_blit_gliss,   // half transparency
    e_blit_mask,    // white masked by key
};

struct blit_info_t {
    const struct bitmap_t * bitmap_;
    vec2i_t dst_pos_;
    recti_t src_rect_;
    blit_type_t type_;
    bool h_flip_;
};

struct font_t {
    const struct bitmap_t * bitmap_;
    int32_t cellw_, cellh_;
    int32_t spacing_;
};

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
        return * target_;
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
