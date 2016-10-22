#pragma once
#include <vector>
#include <map>
#include <set>
#include <SDL/SDL_opengl.h>

#include "../framework_core/vec2.h"
#include "../framework_core/vec3.h"
#include "../framework_core/rect.h"
#include "../framework_core/buffer.h"
#include "../framework_core/mat2.h"
#include "../framework_draw/bitmap.h"

#include "gl_mat4.h"
#include "gl_shader.h"
#include "gl_texture.h"

namespace tengu {
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
    bool bind_attriute_(const int32_t loc, const int32_t count, const float * data);

    std::vector<float> tris_;
    std::vector<float> uv_;
    std::vector<int32_t> index_;
    std::set<int32_t> attrib_;

    gl_texture_t * texture_;
    gl_shader_t * shader_;

    mat4f_t mat_;
    rectf_t viewport_;
};
} // namespace tengu
