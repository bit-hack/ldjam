#include "../external/glee/GLee.h"
#include "gl_draw.h"

namespace tengu {
gl_draw_t::gl_draw_t(const vec2i_t & size) {
}

gl_draw_t::~gl_draw_t() {
}

bool gl_draw_t::clear() {
    return false;
}

bool gl_draw_t::bind(gl_shader_t &shader) {
    dispatch_();
    shader_ = & shader;
#if 0
    glUseProgram( program );
#endif
    return true;
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

bool gl_draw_t::viewport(const rectf_t & rect) {
    return false;
}
} // namespace tengu
