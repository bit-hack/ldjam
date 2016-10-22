#pragma once
#include <cstdint>
#include "../framework_core/rect.h"

namespace tengu {
struct bitmap_t;

struct gl_texture_t {
    gl_texture_t();
    ~gl_texture_t();

    bool load(const bitmap_t &);
    bool store(bitmap_t &);

    bool copy_from(const rectf_t &dst);

    void release();

    operator bool () const;

protected:
    uint32_t id_;
};
} // namespace tengu
