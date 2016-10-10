#include "gl_draw.h"

namespace tengu {
bool gl_shader_t::load(const buffer_t &vert, const buffer_t &frag) {
    return false;
}

bool gl_texture_t::copy_from(struct bitmap_t &bmp, const rectf_t &area) {
    return false;
}

bool gl_texture_t::copy_to(struct bitmap_t &bmp, const rectf_t &area) {
    return false;
}

bool gl_draw_t::bind(gl_shader_t &shader) {
    dispatch_();
    shader_ = & shader;
    return false;
}

bool gl_draw_t::quad(gl_quad_info_t &quad) {
    return false;
}

bool gl_draw_t::line(const vec2f_t &a, const vec2f_t &b) {
    return false;
}

bool gl_draw_t::rect(const rectf_t &rect) {
    return false;
}

bool gl_draw_t::copy_rect(const recti_t &rect, gl_texture_t &dst) {
    return false;
}

bool gl_draw_t::present() {
    return false;
}

bool gl_draw_t::dispatch_() {
    return false;
}
} // namespace tengu
