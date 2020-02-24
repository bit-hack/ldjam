#pragma once
#include <cmath>
#include <initializer_list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "../framework_core/buffer.h"
#include "../framework_core/mat2.h"
#include "../framework_core/rect.h"
#include "../framework_core/vec2.h"
#include "../framework_core/vec4.h"
#include "../framework_draw/bitmap.h"

#include "gl_mat4.h"
#include "gl_shader.h"
#include "gl_texture.h"

#include "../external/glee/GLee.h"

namespace tengu {

struct gl_draw_t;
struct gl_batch_t;
struct gl_batch_quad_t;
struct gl_texture_t;

struct gl_quad_t {
    friend struct gl_batch_quad_t;
    friend struct gl_draw_t;

    enum flag_t {
        e_mirror_u = 1,
        e_mirror_v = 2,
    };

    // default quad
    gl_quad_t()
        : pos_{ 0.f, 0.f, 0.f }
        , frame_{ 0.f, 0.f, 1.f, 1.f }
        , size_{ 1.f, 1.f }
        , angle_{ 0.f }
        , origin_{ .0f, .0f }
    {
    }

    // screen space position in pixels
    gl_quad_t& pos(const vec3f_t& p)
    {
        pos_ = p;
        return *this;
    }

    // rotation angle in radians
    gl_quad_t& angle(const float rot)
    {
        angle_ = rot;
        return *this;
    }

    // note: non normalized uvs
    gl_quad_t& frame(const rectf_t& frame)
    {
        // todo: make frame not defined in UV space
        // 1:    this can be done by normalizing in the vertex shader
        frame_ = frame;
        return *this;
    }

    // note: normalized coordinate
    gl_quad_t& origin(const vec2f_t& origin)
    {
        origin_ = origin;
        return *this;
    }

    // size in screen pixels
    gl_quad_t& size(const vec2f_t& size)
    {
        size_ = size;
        return *this;
    }

    gl_quad_t& flags(uint8_t flags)
    {
        flags_ = flags;
        return *this;
    }

    gl_quad_t& texture(gl_texture_t* tex)
    {
        texture_ = tex;
        return *this;
    }

protected:
    gl_texture_t* texture_;
    uint8_t flags_; // flags bit field
    rectf_t frame_; // texture frame
    vec3f_t pos_; // quad position
    vec2f_t size_; // size
    float angle_; // rotation
    vec2f_t origin_; // position and rotation offset
};

struct gl_depth_t {
    enum {
        e_always,
        e_less,
    } mode_;
    float clear_;
};

struct gl_blend_t {
    enum {
        e_none,
        e_alpha,
        e_add,
    } mode_;
};

struct gl_vertex_t {
    vec4f_t p_;
    vec2f_t t_;
};

// batch for arbitary vertices and indices
struct gl_batch_t {

    gl_batch_t()
    {
    }

    enum {
        e_line,
        e_point,
        e_triangle,
    } mode_;

    void push_vertex(const std::initializer_list<float>& vert)
    {
        assert(vert.size() % 6 == 0);
        const float* base = &(*vert.begin());
        for (size_t i = 0; i < vert.size(); i += 6) {
            const float* f = base + i;
            const vec4f_t p = { f[0], f[1], f[2], f[3] };
            const vec2f_t t = { f[4], f[5] };
            pos_.push_back(p);
            tex_.push_back(t);
        }
    }

    void push_index(const std::initializer_list<int32_t>& index, const size_t base)
    {
        assert(index.size() % 3 == 0);
        for (int32_t i : index) {
            index_.push_back(base + i);
        }
    }

    void clear()
    {
        pos_.clear();
        tex_.clear();
        index_.clear();
    }

    size_t num_vertices() const
    {
        return pos_.size();
    }

    size_t num_indices() const
    {
        return index_.size();
    }

    void resize(size_t num_vertices, size_t num_indices)
    {
        pos_.resize(num_vertices);
        tex_.resize(num_vertices);
        index_.resize(num_indices);
    }

    vec4f_t* data_pos(size_t index = 0)
    {
        return pos_.data() + index;
    }

    vec2f_t* data_tex(size_t index = 0)
    {
        return tex_.data() + index;
    }

    int32_t* data_index(size_t index = 0)
    {
        return index_.data() + index;
    }

    gl_texture_t* texture_;

protected:
    friend struct gl_draw_t;

    std::vector<vec4f_t> pos_;
    std::vector<vec2f_t> tex_;
    std::vector<int32_t> index_;
};

// batch specialised for quads
struct gl_batch_quad_t {
    friend struct gl_draw_t;

    static const size_t COUNT_V = 4;
    static const size_t COUNT_I = 6;

    gl_batch_quad_t()
        : scale_(1.f, 1.f)
        , offset_(-1.f, -1.f)
    {
    }

    // resize the batch
    void resize(size_t size)
    {
        batch_.resize(size * COUNT_V, size * COUNT_I);
    }

    // set a specific quad in the batch
    void set(const size_t slot, const gl_quad_t& quad)
    {
        // todo:
    }

    // return the underlying batch object
    operator gl_batch_t&()
    {
        return batch_;
    }

    // set the overall scale of the batch
    void set_scale(vec2f_t screen)
    {
        scale_ = vec2f_t{
            2.f / screen.x,
            2.f / screen.y
        };
    }

    gl_texture_t* texture_;

protected:
    gl_batch_t batch_;
    vec2f_t scale_;
    vec2f_t offset_;
};

struct gl_camera_t {

    gl_camera_t(const vec2f_t& screen)
        : screen_(screen)
        , pos_{ 0.f, 0.f }
        , zoom_(1.f)
        , rotate_(0.f)
    {
    }

    void get(mat4f_t& out) const
    {
        // dc to ndc scalars
        const float sx = 2.f / screen_.x;
        const float sy = 2.f / screen_.y;
        // output matrix
        out = mat4f_t{
            sx, 0.f, 0.f, 0.f,
            0.f, sy, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f,
        };
    }

    vec2f_t screen_;
    vec2f_t pos_;
    float zoom_;
    float rotate_;
};

struct gl_draw_t {

    gl_draw_t(const vec2i_t& size);
    ~gl_draw_t();

    // clear the framebuffer
    bool clear();

    // bind to the rendering pipeline
    bool bind(gl_shader_t& shader);
    bool bind(const gl_blend_t& blend);
    bool bind(const gl_depth_t& depth);
    bool bind(const gl_camera_t& camera);

    // render elements
    bool draw(const gl_quad_t& quad);
    bool draw(const gl_batch_t& batch);

    // present to the framebuffer
    bool present();
    // sent the renderable viewport
    bool viewport(const rectf_t& rect);
    // flush all pending draw operation
    bool flush();

protected:
    friend struct gl_texture_t;
    friend struct gl_shader_t;

    bool upload_transform();
    bool bind_attriute_(const int32_t loc, const int32_t count, const float* data);

    struct draw_info_t {
        size_t num_vertices_;
        const float* pos_;
        const float* uv_;
        size_t num_indices_;
        const int32_t* index_;
        const gl_texture_t* texture_;
    };

    bool prep_quad_(const gl_quad_t&, vec4f_t * pos, vec2f_t * tex);
    bool draw(const draw_info_t& info);

    // resources
    std::set<int32_t> attrib_;
    std::set<gl_texture_t*> textures_;
    std::set<gl_shader_t*> shaders_;
    // bound states
    gl_depth_t depth_;
    gl_texture_t* texture_;
    gl_shader_t* shader_;
    // projection transform
    mat4f_t transform_;
    rectf_t viewport_;
};
} // namespace tengu
