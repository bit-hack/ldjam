#include "gl_draw.h"
#include "../external/glee/GLee.h"

namespace tengu {
namespace {
static vec2f_t rotate(const float sx, const float sy, const vec2f_t& v)
{
    return vec2f_t{
        sx * v.x + sy * v.y,
        sx * v.y - sy * v.x
    };
}

const auto C_POS = "pos";
const auto C_TEX = "tex";
const auto C_XFORM = "xform";
} // namespace {}

gl_draw_t::gl_draw_t(const vec2i_t& size)
    : texture_(nullptr)
    , shader_(nullptr)
{
    // disable back face culling
    glDisable(GL_CULL_FACE);
    // make sure texturing is enabled
    glEnable(GL_TEXTURE_2D);
    // disable depth test by default
    {
        gl_depth_t depth;
        depth.mode_ = gl_depth_t::e_always;
        depth.clear_ = 0.f;
        bind(depth);
    }
    // load an identity matrix
    transform_.identity();
}

gl_draw_t::~gl_draw_t()
{
}

bool gl_draw_t::clear()
{
    glClearColor(.0f, .1f, .2f, .3f);
    // clear the gl framebuffer
    GLint flags = GL_COLOR_BUFFER_BIT;
    if (depth_.mode_ != depth_.e_always) {
        flags |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(depth_.clear_);
    }
    glClear(flags);
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
        glDepthFunc(GL_ALWAYS);
        break;
    case (gl_depth_t::e_less):
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        break;
    default:
        assert(!"unknown depth mode");
    }
    return true;
}

bool gl_draw_t::draw_(const draw_info_t& info)
{
    if (!shader_) {
        return false;
    }
    if (info.texture_) {
        shader_->bind(C_TEX, *info.texture_, 0);
    }
    // upload the current matrix transform
    upload_transform_();
    // push vertex location
    int32_t loc_pos = 0;
    if (!shader_->get_attrib_loc(C_POS, loc_pos)) {
        return false;
    }
    // bind vertex positions
    const size_t num_verts = info.num_vertices_;
    if (!bind_attriute_(loc_pos, 4, info.pos_)) {
        return false;
    }
    // bind textures
    int32_t loc_tex = 0;
    const bool has_tex = shader_->get_attrib_loc(C_TEX, loc_tex);
    if (has_tex) {
        if (!bind_attriute_(loc_tex, 2, info.uv_)) {
            return false;
        }
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
    glDrawElements(
        GL_TRIANGLES,
        info.num_indices_,
        GL_UNSIGNED_INT,
        info.index_);
    return true;
}

void gl_quad_t::emit_(vec4f_t * pos, vec2f_t * tex) const
{
    const gl_quad_t& quad = *this;
    // rotation angles
    const float sx = sinf(quad.angle_);
    const float cx = cosf(quad.angle_);
    // position
    const float x = quad.pos_.x;
    const float y = quad.pos_.y;
    const float z = quad.pos_.z;
    // offset
    const float ox = -quad.origin_.x * quad.size_.x;
    const float oy = -quad.origin_.y * quad.size_.y;
    // 2d rotation positions
    const std::array<vec2f_t, 4> tmp = {
        rotate(sx, cx, vec2f_t(ox, oy)),
        rotate(sx, cx, vec2f_t(ox + quad.size_.x, oy)),
        rotate(sx, cx, vec2f_t(ox, oy + quad.size_.y)),
        rotate(sx, cx, vec2f_t(ox + quad.size_.x, oy + quad.size_.y)),
    };
    // 4d vertex positions
    pos[0] = vec4f_t{ x + tmp[0].x, y + tmp[0].y, z, 1.f };
    pos[1] = vec4f_t{ x + tmp[1].x, y + tmp[1].y, z, 1.f };
    pos[2] = vec4f_t{ x + tmp[2].x, y + tmp[2].y, z, 1.f };
    pos[3] = vec4f_t{ x + tmp[3].x, y + tmp[3].y, z, 1.f };
    // texture coordinates
    // todo: insert frame dependant coordinates here
    tex[0] = vec2f_t{ 0.f, 0.f };
    tex[1] = vec2f_t{ 1.f, 0.f };
    tex[2] = vec2f_t{ 0.f, 1.f };
    tex[3] = vec2f_t{ 1.f, 1.f };
    // mirror u coordinates
    if (quad.flags_ & gl_quad_t::e_mirror_u) {
        std::swap(tex[0].x, tex[1].x);
        std::swap(tex[2].x, tex[3].x);
    }
    // mirror v coordinates
    if (quad.flags_ & gl_quad_t::e_mirror_v) {
        std::swap(tex[0].y, tex[1].y);
        std::swap(tex[2].y, tex[3].y);
    }
}

bool gl_draw_t::draw(const gl_quad_t& quad)
{
    if (!shader_) {
        return false;
    }
    if (quad.texture_) {
        gl_texture_t* tex = quad.texture_;
        shader_->bind(C_TEX, *tex, 0);
    }
    // 4d vertex positions
    std::array<vec4f_t, 4> pos;
    // texture coordinates
    std::array<vec2f_t, 4> tex;
    quad.emit_(pos.data(), tex.data());
    // index array
    const std::array<int32_t, 6> index = {
        0, 1, 2,
        1, 3, 2
    };
    // setup draw info
    draw_info_t draw_info = {
        pos.size(),
        &pos.data()->x,
        &tex.data()->x,
        index.size(),
        index.data(),
        quad.texture_
    };
    // todo: we can use this code to insert into a batch too
    return draw_(draw_info);
}

bool gl_draw_t::draw(const gl_batch_t& batch)
{
    draw_info_t draw_info = {
        batch.num_vertices(),
        &(batch.pos_.data()->x),
        &(batch.tex_.data()->x),
        batch.index_.size(),
        batch.index_.data(),
        nullptr
    };
    return draw_(draw_info);
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
    default:
        assert(!"unknown blend mode");
    }
    return true;
}

bool gl_draw_t::bind(const gl_camera_t& camera)
{
    camera.get(transform_);
    upload_transform_();
    return true;
}

bool gl_draw_t::upload_transform_()
{
    return shader_ ? shader_->bind(C_XFORM, transform_) : false;
}

bool gl_draw_t::flush()
{
    glFlush();
    return true;
}

bool gl_draw_t::viewport(const rectf_t& rect)
{
    glViewport(
        GLint(rect.x0),
        GLint(rect.y0),
        GLint(rect.width()),
        GLint(rect.height()));
    return true;
}
} // namespace tengu
