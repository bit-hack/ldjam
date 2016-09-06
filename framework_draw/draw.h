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
    struct bitmap_t * bitmap_;
    vec2i_t dst_pos_;
    recti_t src_rect_;
    blit_type_t type_;
    bool h_flip_;
};

struct triangle_t {
    vec3f_t p0_, p1_, p2_;
    vec2f_t t0_, t1_, t2_;
    const bitmap_t & tex_;
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

    void triangle(const triangle_t &);

    void line(const vec2f_t & p0,
              const vec2f_t & p1);

    void line(const vec2i_t & p0,
              const vec2i_t & p1);

    void plot(const vec2i_t & p);

    void blit(const blit_info_t & info);

    void set_target(struct bitmap_t &);

    void viewport(const recti_t & rect);

    void viewport();

    void copy(struct bitmap_t & dst,
              const recti_t & src_rect,
              const vec2i_t & dst_pos);

    bitmap_t & get_target() {
        return * target_;
    }

    void render_2x(void *, const uint32_t pitch);

    uint32_t colour_;

protected:

    void _span(int32_t x0, int32_t x1, int32_t y);
    void _clip(vec2i_t & p0, vec2i_t & p1);
//    void _clip(recti_t & src, recti_t & dst);

    recti_t _target_size() const;

    recti_t viewport_;
    bitmap_t * target_;
};
