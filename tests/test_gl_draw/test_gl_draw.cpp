#include <array>
#include <SDL/SDL.h>

#include "../../framework_core/vec2.h"
#include "../../external/glee/GLee.h"
#include "../../framework_gl_draw/gl_draw.h"

const char vshad[] = R"(
attribute vec4 pos;
attribute vec2 tex;
varying vec2 tex_out;
void main() {
    tex_out =  tex;
    gl_Position = pos;
})";

const char fshad[] = R"(
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
        bmp.load("D:/projects/ldjam/tests/assets/sprite1.bmp");
        tengu::gl_texture_t texture;
        texture.load(bmp);

        tengu::buffer_t vs(vshad), fs(fshad);

        tengu::gl_shader_t shader;
        if (!shader.load(vs, fs)) {
            return -1;
        }

        draw.bind(shader);

        shader.bind("texture1", texture, 0);

        while (active_) {
            tick_events();
            draw.clear();
            tengu::gl_quad_info_t info;
            draw.quad(info);
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
