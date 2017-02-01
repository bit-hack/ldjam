#include <SDL/SDL.h>
#include <array>

#include "../../external/glee/GLee.h"
#include "../../framework_core/vec2.h"
#include "../../framework_gl_draw/gl_draw.h"

namespace {
static const char vshad[] = R"(
#version 130
attribute vec4 pos;
attribute vec2 tex;
varying vec2 tex_out;
uniform mat4 xform;
void main() {
    tex_out =  tex;
    gl_Position = pos * xform;
})";

static const char fshad[] = R"(
#version 130
varying vec2 tex_out;
uniform sampler2D texture1;
void main() {
    gl_FragColor = texture(texture1, tex_out);
})";

SDL_Surface* screen_ = nullptr;
} // namespace {}

bool init()
{
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
        : active_(true)
    {
    }

    void tick_events()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    active_ = false;
                }
            }
            if (event.type == SDL_QUIT) {
                active_ = false;
            }
        }
    }

    int main(const int argc, char* args[])
    {
        if (!init()) {
            return 1;
        }
        
        // create a gl draw instance
        tengu::gl_draw_t draw(tengu::vec2i_t{ screen_->w, screen_->h });

        // load up the test sprite
        tengu::bitmap_t bmp;
        bmp.load(TENGU_TEST_DIR "/../assets/sprite1.bmp");
        if (!bmp) {
            return 1;
        }
        bmp.colour_key(0x0);

        // bind a camera
        tengu::gl_camera_t camera(tengu::vec2f_t{
            screen_->w,
            screen_->h});
        draw.bind(camera);

        // load us some shaders
        tengu::buffer_t vs(vshad), fs(fshad);
        tengu::gl_shader_t shader(draw);
        if (!shader.load(vs, fs)) {
            return -1;
        }
        draw.bind(shader);

        // load us some textures
        tengu::gl_texture_t texture(draw);
        if (!texture.load(bmp)) {
            return 1;
        }
        shader.bind("texture1", texture, 0);

        // set blend mode to alpha
        draw.bind(tengu::gl_blend_t{ tengu::gl_blend_t::e_alpha });

        // create (unused) batch
        tengu::gl_batch_t batch;
        batch.push_vertex({
            -1.f, -1.f, 0.f, 1.f, 0.f, 0.f,
             1.f, -1.f, 0.f, 1.f, 1.f, 0.f,
            -1.f,  1.f, 0.f, 1.f, 0.f, 1.f,
             1.f,  1.f, 0.f, 1.f, 1.f, 1.f });
        batch.push_index({
            0, 1, 2,
            1, 3, 2 },
            0);

        // create a quad batch
        tengu::gl_batch_quad_t qb;
        qb.resize(2);
        // frame mat pos
        tengu::gl_quad_t q1 = tengu::gl_quad_t()
            .pos(tengu::vec3f_t{ 0.f, 0.f, 0.f })
            .size(tengu::vec2f_t{64.f, 64.f});
        tengu::gl_quad_t q2 = tengu::gl_quad_t()
            .size(tengu::vec2f_t{32.f, 32.f})
            .pos(tengu::vec3f_t{ 0.f, 0.f, 0.f });

        float a = 0.f;

        // main loop
        while (active_) {
            tick_events();
            draw.clear();
            draw.draw(q1);
            draw.draw(q2);

            q1.origin(tengu::vec2f_t{.5f, .5f});
            q1.angle(a);
            a += 0.05f;

            draw.present();
            SDL_GL_SwapBuffers();
            SDL_Delay(1000 / 25);
        }
        return 0;
    }

    bool active_;
};

int main(int argc, char** args)
{
    main_t main_class;
    return main_class.main(argc, args);
}
