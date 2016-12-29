#pragma once
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <cmath>
#include <initializer_list>

#include <SDL/SDL_opengl.h>

#include "../framework_core/vec2.h"
#include "../framework_core/vec4.h"
#include "../framework_core/rect.h"
#include "../framework_core/buffer.h"
#include "../framework_core/mat2.h"
#include "../framework_draw/bitmap.h"

#include "gl_mat4.h"
#include "gl_shader.h"
#include "gl_texture.h"

namespace tengu {
struct gl_quad_t {

    gl_quad_t()
        : pos_{ 0.f, 0.f, 0.f}
        , frame_{0.f, 0.f,
                 1.f, 1.f}
        , mat_{1.f, 0.f,
               0.f, 1.f}
    {
    }

    gl_quad_t(const vec3f_t & pos, const float & rot, const float scale)
            : pos_(pos)
            , frame_{0.f, 0.f,
                     1.f, 1.f}
            , mat_{scale*cosf(rot), scale*-sinf(rot),
                   scale*sinf(rot), scale* cosf(rot)}
    {
    }

    gl_quad_t(const vec3f_t & pos, rectf_t frame)
        : pos_(pos)
        , frame_{frame.x0, frame.y0,
                 frame.x1, frame.y1}
        , mat_{1.f, 0.f,
               0.f, 1.f}
    {
    }

    vec3f_t pos_;
    rectf_t frame_;
    mat2f_t mat_;
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

struct gl_batch_t {

    gl_batch_t()
    {
    }

    enum {
        e_line,
        e_point,
        e_triangle,
    } mode_;

    void push_vertex(const std::initializer_list<float> & vert) {
        assert(vert.size() % 6 == 0);
        const float * base = &(*vert.begin());
        for (size_t i=0; i<vert.size(); i+=6) {
            const float *f = base + i;
            const vec4f_t p = {f[0], f[1], f[2], f[3]};
            const vec2f_t t = {f[4], f[5]};
            pos_.push_back(p);
            tex_.push_back(t);
        }
    }

    void push_index(const std::initializer_list<int32_t> & index, const size_t base) {
        assert(index.size() % 3 == 0);
        for (int32_t i : index) {
            index_.push_back(base + i);
        }
    }

    void clear() {
        pos_.clear();
        tex_.clear();
        index_.clear();
    }

    size_t num_vertices() const {
        return pos_.size();
    }

    size_t num_indices() const {
        return index_.size();
    }

    void resize(size_t num_vertices, size_t num_indices) {
        pos_.resize(num_vertices);
        tex_.resize(num_vertices);
        index_.resize(num_indices);
    }

    vec4f_t * data_pos(size_t index = 0) {
        return pos_.data() + index;
    }

    vec2f_t * data_tex(size_t index = 0) {
        return tex_.data() + index;
    }

    int32_t * data_index(size_t index = 0) {
        return index_.data() + index;
    }

protected:
    friend struct gl_draw_t;

    std::vector<vec4f_t> pos_;
    std::vector<vec2f_t> tex_;
    std::vector<int32_t> index_;
};

struct gl_batch_quad_t {

    gl_batch_quad_t()
        : scale_(1.f, 1.f)
        , offset_(-1.f, -1.f)
    {
    }

    void resize(size_t size) {
        batch_.resize(size * 4, size * 6);
    }

    vec4f_t xform(const mat2f_t & m, const vec3f_t & p, const vec2f_t & in) {
        return vec4f_t {
            offset_.x + p.x + (in.x * m[0] + in.y * m[1]) * scale_.x,
            offset_.y + p.y + (in.x * m[2] + in.y * m[3]) * scale_.y,
            p.z,
            1.f
        };
    }

    void set(const size_t slot, const gl_quad_t & quad) {
        vec4f_t * p = batch_.data_pos(slot * 4);
        vec2f_t * t = batch_.data_tex(slot * 4);
        int32_t * i = batch_.data_index(slot * 6);
        const int32_t base = int32_t(slot * 4);
        // transform by mat2
        p[0] = xform(quad.mat_, quad.pos_, vec2f_t{-1.f, -1.f});
        p[1] = xform(quad.mat_, quad.pos_, vec2f_t{ 1.f, -1.f});
        p[2] = xform(quad.mat_, quad.pos_, vec2f_t{-1.f,  1.f});
        p[3] = xform(quad.mat_, quad.pos_, vec2f_t{ 1.f,  1.f});
        // update tex cords
        t[0] = vec2f_t{quad.frame_.x0, quad.frame_.y0};
        t[1] = vec2f_t{quad.frame_.x1, quad.frame_.y0};
        t[2] = vec2f_t{quad.frame_.x0, quad.frame_.y1};
        t[3] = vec2f_t{quad.frame_.x1, quad.frame_.y1};
        // set indices
        i[0] = base + 0; i[1] = base + 1; i[2] = base + 2;
        i[3] = base + 1; i[4] = base + 3; i[5] = base + 2;
    }

    operator gl_batch_t & () {
        return batch_;
    }

    void set_scale(vec2f_t screen) {
        scale_ = vec2f_t{2.f / screen.x,
                         2.f / screen.y};
    }

    gl_batch_t batch_;
    vec2f_t scale_;
    vec2f_t offset_;
};

struct gl_draw_t {

    gl_draw_t(const vec2i_t & size);
    ~gl_draw_t();

    bool clear();

    bool bind(gl_shader_t & shader);
    bool bind(const gl_blend_t & blend);
    bool bind(const gl_depth_t & depth);

    bool draw(const gl_batch_t & batch);

    bool present();
    bool viewport(const rectf_t & rect);
    bool flush();

protected:
    friend struct gl_texture_t;
    friend struct gl_shader_t;

    bool bind_attriute_(const int32_t loc, const int32_t count, const float * data);

    std::set<int32_t> attrib_;

    std::set<gl_texture_t*> textures_;
    std::set<gl_shader_t*> shaders_;

    gl_depth_t depth_;
    gl_texture_t * texture_;
    gl_shader_t * shader_;

    rectf_t viewport_;
};
} // namespace tengu
