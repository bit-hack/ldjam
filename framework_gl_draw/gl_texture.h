#pragma once
#include <cstdint>
#include "../framework_core/rect.h"

namespace tengu {
struct bitmap_t;
struct gl_draw_t;

struct gl_texture_t {
    gl_texture_t(gl_draw_t &);
    ~gl_texture_t();

    bool load(const bitmap_t &);
    bool store(bitmap_t &);

    bool copy_from(const rectf_t &dst);

    void release();

    operator bool () const;

protected:
    friend struct gl_draw_t;
    gl_draw_t & draw_;

    uint32_t id_;
};
} // namespace tengu
