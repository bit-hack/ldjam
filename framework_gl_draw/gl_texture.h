#pragma once
#include <cstdint>
#include "../framework_core/rect.h"

namespace tengu {
struct bitmap_t;

struct gl_texture_t {
    bool copy_from(const bitmap_t &, const rectf_t & src);

    bool copy_to(bitmap_t &, const rectf_t &dst);

protected:
    int32_t id_;
};
} // namespace tengu
