#include "gl_draw.h"
#include "../external/glee/GLee.h"

namespace tengu {
gl_draw_t::gl_draw_t(const vec2i_t& size)
    : texture_(nullptr)
    , shader_(nullptr)
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
}

gl_draw_t::~gl_draw_t()
{
}

bool gl_draw_t::clear()
{
    glClearColor(.0f, .1f, .2f, .3f);
    glClear(GL_COLOR_BUFFER_BIT);

    depth_.mode_ = depth_.e_always;
    depth_.clear_ = 1.f;
    return true;
}

bool gl_draw_t::bind(gl_shader_t& shader)
{
    shader_ = &shader;
    assert(shader.program_ != GL_INVALID_VALUE);
    glUseProgram(shader.program_);
    return true;
}

bool gl_draw_t::bind(const gl_depth_t& test)
{
    depth_ = test;
    switch (test.mode_) {
    case (gl_depth_t::e_always):
        glDisable(GL_DEPTH_TEST);
        break;
    case (gl_depth_t::e_less):
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        break;
    default:
        return false;
    }
    return true;
}

bool gl_draw_t::draw(const gl_batch_t& batch)
{
    if (!shader_) {
        return false;
    }
    // push vertex location
    int32_t loc_pos = 0;
    if (!shader_->get_attrib_loc("pos", loc_pos)) {
        return false;
    }
    // bind vertex positions
    const size_t num_verts = batch.num_vertices();
    bind_attriute_(loc_pos, 4, &(batch.pos_.data()->x));
    // bind textures
    int32_t loc_tex = 0;
    bool has_tex = shader_->get_attrib_loc("tex", loc_tex);
    if (has_tex) {
        bind_attriute_(loc_tex, 2, &(batch.tex_.data()->x));
    }
    // disable any previously bound attributes
    for (auto itt = attrib_.begin(); itt != attrib_.end();) {
        const int32_t loc = *itt;
        bool remove = true;
        remove &= loc != loc_pos;
        remove &= has_tex ? loc != loc_tex : true;
        if (remove) {
            glDisableVertexAttribArray(loc);
            itt = attrib_.erase(itt);
        } else {
            ++itt;
        }
    }
    // draw all of the elements that we have in our buffers
    glDrawElements(GL_TRIANGLES, batch.index_.size(), GL_UNSIGNED_INT, batch.index_.data());
    return true;
}

bool gl_draw_t::present()
{
    return true;
}

bool gl_draw_t::bind_attriute_(const int32_t loc, const int32_t count, const float* data)
{
    assert(data && count >= 1);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, count, GL_FLOAT, GL_FALSE, 0, data);
    attrib_.insert(loc);
    return true;
}

bool gl_draw_t::bind(const gl_blend_t& blend)
{
    switch (blend.mode_) {
    case (gl_blend_t::e_none):
        glDisable(GL_BLEND);
        break;
    case (gl_blend_t::e_alpha):
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case (gl_blend_t::e_add):
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
    }
    return true;
}

bool gl_draw_t::flush()
{
    return true;
}

bool gl_draw_t::viewport(const rectf_t& rect)
{
    glViewport(rect.x0, rect.y0, rect.width(), rect.height());
    return true;
}
} // namespace tengu
