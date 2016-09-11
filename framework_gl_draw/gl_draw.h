#pragma once
#include <vector>

#include "../framework_core/vec3.h"
#include "../framework_core/rect.h"
#include "../framework_core/buffer.h"

struct mat4_t {
    float e[4*4];
};

struct gl_texture_t {
};

struct gl_shader_t {
    buffer_t vert_;
    buffer_t frag_;
};

struct gl_mesh_t {
    std::vector<vec3f_t> vertices_;
};

struct gl_draw_t {

    bool bind(gl_texture_t & texture);
    bool bind(gl_shader_t & shader);

    bool draw(gl_mesh_t & mesh);

    bool copy_rect(const recti_t & rect, gl_texture_t & dst);

    void present();
};
