#include "../external/glee/GLee.h"
#include "gl_draw.h"

namespace tengu {
gl_draw_t::gl_draw_t(const vec2i_t & size) 
    : texture_(nullptr)
    , shader_(nullptr)
{
}

gl_draw_t::~gl_draw_t() {
}

bool gl_draw_t::clear() {
    glClearColor(.0f, .1f, .2f, .3f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    return false;
}

bool gl_draw_t::bind(gl_shader_t &shader) {
    dispatch_();
    shader_ = & shader;
    assert(shader.program_!=GL_INVALID_VALUE);
    glUseProgram(shader.program_);
    return true;
}

bool gl_draw_t::quad(gl_quad_info_t &quad) {
    // we dont have a bound shader
    if (!shader_) {
        return false;
    }
    // push vertex location
    int32_t loc_pos = 0;
    if (!shader_->get_attrib_loc("pos", loc_pos)) {
        return false;
    }
    const std::array<float, 16> dat_pos = {
        -1.f, -1.f, 0.f, 1.f,
         1.f, -1.f, 0.f, 1.f,
         1.f,  1.f, 0.f, 1.f,
        -1.f,  1.f, 0.f, 1.f,
    };
    tris_.insert(tris_.end(), dat_pos.begin(), dat_pos.end());
    // push any texture coordinates we have
    int32_t loc_tex = -0;
    bool has_tex = shader_->get_attrib_loc("tex", loc_tex);
    if (has_tex) {
        const std::array<float, 16> dat_tex = {
            0.f, 0.f,
            1.f, 0.f,
            1.f, 1.f,
            0.f, 1.f,
        };
        uv_.insert(uv_.end(), dat_tex.begin(), dat_tex.end());
    }
    // push back out indices
    const int32_t base(index_.size());
    // tri 1
    index_.push_back(base+0);
    index_.push_back(base+1);
    index_.push_back(base+2);
    // tri 2
    index_.push_back(base+0);
    index_.push_back(base+2);
    index_.push_back(base+3);
    return true;
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
    dispatch_();
    return true;
}

bool gl_draw_t::bind_attriute_(const int32_t loc, const int32_t count, const float * data) {
    assert(data && count>=1);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, count, GL_FLOAT, GL_FALSE, 0, data);
    attrib_.insert(loc);
    return true;
}

bool gl_draw_t::dispatch_() {
    // we dont have a bound shader
    if (!shader_) {
        return false;
    }
    // nothing to dispatch
    if (index_.empty()) {
        return true;
    }
    // find vertex position attribute location
    int32_t loc_pos = 0;
    const bool has_pos = shader_->get_attrib_loc("pos", loc_pos);
    if (!has_pos) {
        return false;
    }
    bind_attriute_(loc_pos, 4, tris_.data());
    // find vertex texture coordinate position
    int32_t loc_tex = -1;
    const bool has_tex = shader_->get_attrib_loc("tex", loc_tex);
    // bind attributes
    if (has_tex) {
        bind_attriute_(loc_tex, 2, uv_.data());
    }
    // disable any previously bound attributes
    for (auto itt = attrib_.begin(); itt!=attrib_.end();) {
        const int32_t loc = *itt;
        bool remove = true;
        remove &= loc!=loc_pos;
        remove &= has_tex ? loc!=loc_tex : true;
        if (remove) {
            glDisableVertexAttribArray(loc);
            itt = attrib_.erase(itt);
        }
        else {
            ++itt;
        }
    }
    // draw all of the elements that we have in out buffers
    glDrawElements(GL_TRIANGLES, index_.size(), GL_UNSIGNED_INT, index_.data());
    // clear the buffers we have
    tris_.clear();
    uv_.clear();
    index_.clear();
    return true;
}

bool gl_draw_t::viewport(const rectf_t & rect) {
    return false;
}
} // namespace tengu
