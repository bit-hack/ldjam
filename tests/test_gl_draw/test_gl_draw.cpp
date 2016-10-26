#include <array>
#include <SDL/SDL.h>

#include "../../framework_core/vec2.h"
#include "../../external/glee/GLee.h"
#include "../../framework_gl_draw/gl_draw.h"

const char vshad[] = R"(
#version 130
attribute vec4 pos;
attribute vec2 tex;
varying vec2 tex_out;
void main() {
    tex_out =  tex;
    gl_Position = pos;
})";

const char fshad[] = R"(
#version 130
varying vec2 tex_out;
uniform sampler2D texture1;
void main() {
    gl_FragColor = texture(texture1, tex_out);
})";

namespace {
    SDL_Surface * screen_;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

    screen_ = SDL_SetVideoMode(640, 480, 32, SDL_OPENGL);
    if (!screen_) {
        return false;
    }
    return true;
}

struct main_t {

    main_t() 
        : active_(true )
    {
    }

    void tick_events() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            if (event.type==SDL_KEYDOWN) {
                if (event.key.keysym.sym==SDLK_ESCAPE) {
                    active_ = false;
                }
            }
            if (event.type==SDL_QUIT) {
                active_ = false;
            }
        }
    }

    int main(const int argc, char *args[]) {
        if (!init()) {
            return 1;
        }

        tengu::gl_draw_t draw(tengu::vec2i_t{screen_->w, screen_->h});

        tengu::bitmap_t bmp;
        bmp.load("/home/flipper/repos/ldjam/tests/assets/sprite1.bmp");
        if (!bmp) {
            return 1;
        }
        bmp.colour_key(0x0);

        tengu::gl_texture_t texture(draw);
        if (!texture.load(bmp)) {
            return 1;
        }

        tengu::buffer_t vs(vshad), fs(fshad);

        tengu::gl_shader_t shader(draw);
        if (!shader.load(vs, fs)) {
            return -1;
        }

        draw.bind(tengu::gl_blend_t{tengu::gl_blend_t::e_alpha});
        draw.bind(shader);
        shader.bind("texture1", texture, 0);

        tengu::gl_batch_t batch;
        batch.push_vertex({
            -1.f,-1.f, 0.f, 1.f, 0.f, 0.f,
             1.f,-1.f, 0.f, 1.f, 1.f, 0.f,
            -1.f, 1.f, 0.f, 1.f, 0.f, 1.f,
             1.f, 1.f, 0.f, 1.f, 1.f, 1.f});

        batch.push_index({
            0, 1, 2,
            1, 3, 2}, 0);

        tengu::gl_batch_quad_t qb;
        qb.resize(2);
        // frame mat pos
        qb.set(0, tengu::gl_quad_t(tengu::vec3f_t {0.f, 0.f, 0.f}, tengu::rectf_t{.5f, .5f, .1f, .1f}));
        qb.set(1, tengu::gl_quad_t(tengu::vec3f_t {0.f, 0.f, 0.f}, 0.1f, 0.1f));

        while (active_) {
            tick_events();
            draw.clear();
            draw.draw(qb);
            draw.present();
            SDL_GL_SwapBuffers();
            SDL_Delay(1000/25);
        }
        return 0;
    }

    bool active_;
};

int main(int argc, char ** args) {
    main_t main_class;
    return main_class.main(argc, args);
}
