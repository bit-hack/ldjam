#include "../external/glee/GLee.h"
#include "gl_texture.h"
#include "../framework_draw/bitmap.h"
#include "../framework_core/rect.h"

namespace tengu {
bool gl_texture_t::copy_from(const bitmap_t &, const rectf_t & src) {
    return false;
}

bool gl_texture_t::copy_to(bitmap_t &, const rectf_t &dst) {
    return false;
}
} // namespace tengu
