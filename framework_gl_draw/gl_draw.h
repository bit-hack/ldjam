#pragma once
#include <vector>

#include "../framework_core/vec2.h"
#include "../framework_core/vec3.h"
#include "../framework_core/rect.h"
#include "../framework_core/buffer.h"

namespace tengu {
struct mat4_t {
    float e[4*4];
};

struct gl_texture_t {
    bool load(const char *);
    bool save(const char *);

    void copy_from(struct bitmap_t *);
    void copy_to(struct bitmap_t *);

    void update(struct bitmap_t *,
                const vec2i_t & src,
                const vec2i_t & dst,
                const vec2i_t & size);
};

struct gl_shader_t {
    bool load(const char *);
protected:
    tengu::buffer_t vert_;
    tengu::buffer_t frag_;
};

struct gl_mesh_t {
    bool load(const char *);
protected:
    std::vector<vec3f_t> pos_;
    std::vector<vec3f_t> rgb_;
    std::vector<vec2f_t> uv_;
};

struct gl_draw_t {
    bool bind(gl_texture_t & texture);
    bool bind(gl_shader_t & shader);

    bool draw(gl_mesh_t & mesh);

    bool copy_rect(const recti_t & rect, gl_texture_t & dst);

    void present();
};
} // namespace tengu
