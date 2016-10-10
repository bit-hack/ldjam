#pragma once
#include <vector>
#include <map>
#include <SDL/SDL_opengl.h>

#include "../framework_core/vec2.h"
#include "../framework_core/vec3.h"
#include "../framework_core/rect.h"
#include "../framework_core/buffer.h"
#include "../framework_core/mat2.h"
#include "../framework_draw/bitmap.h"

namespace tengu {
template <typename type_t>
struct mat4_t {
    std::array<type_t, 16> e_;
};

typedef mat4_t<float> mat4f_t;

struct gl_texture_t {
    bool copy_from(struct bitmap_t &, const rectf_t & src);
    bool copy_to(struct bitmap_t &, const rectf_t & dst);
protected:
    int32_t id_;
};

struct gl_shader_t {
    bool load(const buffer_t & vert,
              const buffer_t & frag);

    bool bind(const char * name, gl_texture_t & val) {
        return false;
    }

    bool bind(const char * name, mat4f_t & val);
    bool bind(const char * name, mat2f_t & val);
    bool bind(const char * name, vec3f_t & val);
    bool bind(const char * name, vec2f_t & val);
    bool bind(const char * name, float & val);

protected:
    int32_t program_;
};

struct gl_quad_info_t {
    rectf_t frame_;
    vec3f_t pos_;
    mat2f_t mat_;
};

struct gl_draw_t {

    gl_draw_t(const vec2i_t & size);
    ~gl_draw_t();

    bool clear();
    bool bind(gl_shader_t & shader);
    bool quad(gl_quad_info_t & quad);
    bool line(const vec2f_t & a, const vec2f_t & b);
    bool rect(const rectf_t & rect);
    bool copy_rect(const recti_t & rect, gl_texture_t & dst);
    bool present();
    bool viewport(const rectf_t & rect);

protected:
    bool dispatch_();

    std::vector<float> tris_;
    std::vector<float> uv_;
    gl_texture_t * texture_;
    gl_shader_t * shader_;
//    mat4_t mat_;
    rectf_t viewport_;
};
} // namespace tengu
