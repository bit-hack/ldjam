#include <array>
#include <SDL/SDL.h>

#include "../../external/glee/GLee.h"
#include "../../framework_gl_draw/gl_draw.h"

const char vshad[] = R"(
attribute vec4 vPosition;
//attribute vec3 vNormal;
//attribute vec2 vTexCoord;
//uniform mat4 vTransform;
//uniform mat4 vProj;
//varying vec3 sNormal;
//varying vec2 sUV;
void main() {
//    sUV =  vTexCoord;
//    gl_Position = (vPosition * vTransform) * vProj;
        gl_Position = vPosition;
//    sNormal = vNormal;
})";

const char fshad[] = R"(
//uniform sampler2D tex0;
//varying vec3 sNormal;
//varying vec2 sUV;
void main() {
    // stored as rgba
//    float flux = sNormal.z * .5 + .5;
//    gl_FragColor = vec4( sNormal.xyz, 0.0f );
    gl_FragColor = vec4( 1.f, 1.f, 1.f, 1.f );
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

        tengu::bitmap_t bmp;
        bmp.load("/home/flipper/repos/ldjam/tests/assets/sprite1.bmp");
        tengu::gl_texture_t texture;
        texture.load(bmp);

        tengu::buffer_t vs(vshad, sizeof(vshad));
        tengu::buffer_t fs(fshad, sizeof(fshad));

        tengu::gl_shader_t shader;
        shader.load(vs, fs);

        while (active_) {
            tick_events();
            glClearColor(.0f, .1f, .2f, .3f);
            glClear(GL_COLOR_BUFFER_BIT);

            glRasterPos2f(.0f, .0f);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_TRIANGLE_FAN);
            glColor3i(0, 0, 0);
            glVertex3f( 0.f, -1.f, .5f);
            glVertex3f(-1.f, 1.f, .5f);
            glVertex3f( 1.f, 1.f, .5f);
            glEnd();

            if (bmp.valid()) {
                glViewport(0, 0, screen_->w, screen_->h);
                glRasterPos2f(.5f, .5f);
                glDrawPixels(bmp.width(), bmp.height(), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, bmp.data());
            }

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
